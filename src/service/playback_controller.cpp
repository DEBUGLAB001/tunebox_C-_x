#include "service/playback_controller.hpp"
#include <algorithm>
#include <random>

namespace tunebox {

PlaybackController::PlaybackController(LibraryService& library, PlaybackQueue& queue)
    : library_(library), queue_(queue) {
    reset();
}

void PlaybackController::record_history(const std::string& track_id) {
    if (track_id.empty()) return;
    // Remove existing occurrence to place at front
    auto it = std::find(history_.begin(), history_.end(), track_id);
    if (it != history_.end()) {
        history_.erase(it);
    }
    history_.push_front(track_id);
    if (history_.size() > MAX_HISTORY) {
        history_.pop_back();
    }
}

bool PlaybackController::play(const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string target_id = track_id;

    if (target_id.empty()) {
        if (!state_.current_track_id.empty()) {
            state_.is_playing = true;
            return true;
        }
        // If no track selected, default to first track in library
        auto all_tracks = library_.get_all_tracks();
        if (all_tracks.empty()) return false;
        target_id = all_tracks.front().id;
    }

    auto track = library_.get_track_by_id(target_id);
    if (!track) return false;

    state_.current_track_id = target_id;
    state_.is_playing = true;
    state_.position_sec = 0;
    state_.current_track = *track;

    library_.increment_play_count(target_id);
    record_history(target_id);
    return true;
}

void PlaybackController::pause() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.is_playing = false;
}

bool PlaybackController::next() {
    std::string next_track_id;

    // Check repeat mode "one"
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (state_.repeat_mode == "one" && !state_.current_track_id.empty()) {
            state_.position_sec = 0;
            state_.is_playing = true;
            return true;
        }
    }

    // 1. Consume from queue first (FIFO)
    next_track_id = queue_.dequeue();

    // 2. If queue is empty, fallback to library order or shuffle
    if (next_track_id.empty()) {
        auto all_tracks = library_.get_all_tracks();
        if (all_tracks.empty()) return false;

        std::lock_guard<std::mutex> lock(mutex_);
        if (state_.shuffle) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<size_t> dis(0, all_tracks.size() - 1);
            size_t idx = dis(gen);
            next_track_id = all_tracks[idx].id;
        } else {
            // Find current index
            size_t curr_idx = 0;
            bool found = false;
            for (size_t i = 0; i < all_tracks.size(); ++i) {
                if (all_tracks[i].id == state_.current_track_id) {
                    curr_idx = i;
                    found = true;
                    break;
                }
            }

            if (!found || curr_idx + 1 >= all_tracks.size()) {
                if (state_.repeat_mode == "all") {
                    next_track_id = all_tracks.front().id;
                } else {
                    // Repeat off and reached end: stop playback
                    state_.is_playing = false;
                    state_.position_sec = 0;
                    return false;
                }
            } else {
                next_track_id = all_tracks[curr_idx + 1].id;
            }
        }
    }

    return play(next_track_id);
}

bool PlaybackController::previous() {
    std::lock_guard<std::mutex> lock(mutex_);

    // If more than 3 seconds in, restart track
    if (state_.position_sec > 3) {
        state_.position_sec = 0;
        return true;
    }

    // Check history (history_[0] is current track, history_[1] is previous)
    if (history_.size() > 1) {
        std::string prev_id = history_[1];
        history_.pop_front(); // remove current
        lock.~lock_guard();
        return play(prev_id);
    }

    // Fallback: previous in library
    auto all_tracks = library_.get_all_tracks();
    if (all_tracks.empty()) return false;

    size_t curr_idx = 0;
    for (size_t i = 0; i < all_tracks.size(); ++i) {
        if (all_tracks[i].id == state_.current_track_id) {
            curr_idx = i;
            break;
        }
    }

    size_t prev_idx = (curr_idx == 0) ? (all_tracks.size() - 1) : (curr_idx - 1);
    std::string prev_id = all_tracks[prev_idx].id;
    lock.~lock_guard();
    return play(prev_id);
}

bool PlaybackController::seek(int position_sec) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_.current_track_id.empty()) return false;
    auto track = library_.get_track_by_id(state_.current_track_id);
    if (!track) return false;

    state_.position_sec = std::max(0, std::min(position_sec, track->duration));
    return true;
}

void PlaybackController::set_volume(int volume) {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.volume = std::max(0, std::min(volume, 100));
}

void PlaybackController::set_repeat_mode(const std::string& mode) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (mode == "off" || mode == "all" || mode == "one") {
        state_.repeat_mode = mode;
    }
}

void PlaybackController::toggle_shuffle() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.shuffle = !state_.shuffle;
}

PlaybackState PlaybackController::get_state() const {
    std::lock_guard<std::mutex> lock(mutex_);
    PlaybackState s = state_;
    if (!s.current_track_id.empty()) {
        auto track = library_.get_track_by_id(s.current_track_id);
        if (track) {
            s.current_track = *track;
        }
    }
    return s;
}

std::vector<Track> PlaybackController::get_history() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Track> list;
    for (const auto& tid : history_) {
        auto t = library_.get_track_by_id(tid);
        if (t) {
            list.push_back(*t);
        }
    }
    return list;
}

void PlaybackController::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    state_.current_track_id = "";
    state_.is_playing = false;
    state_.position_sec = 0;
    state_.volume = 80;
    state_.repeat_mode = "off";
    state_.shuffle = false;
    state_.current_track = nullptr;
    history_.clear();

    auto all_tracks = library_.get_all_tracks();
    if (!all_tracks.empty()) {
        state_.current_track_id = all_tracks.front().id;
        state_.current_track = all_tracks.front();
        history_.push_front(all_tracks.front().id);
    }
}

} // namespace tunebox
