#include "service/playback_queue.hpp"
#include <algorithm>

namespace tunebox {

PlaybackQueue::PlaybackQueue(const LibraryService& library)
    : library_(library) {}

void PlaybackQueue::enqueue(const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    // Verify track exists in library
    if (library_.get_track_by_id(track_id)) {
        queue_.push_back(track_id);
    }
}

std::string PlaybackQueue::dequeue() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return "";
    }
    queue_.pop_front();
    if (queue_.empty()) {
        return "";
    }
    std::string track_id = queue_.front();
    queue_.pop_front();
    return track_id;
}

std::optional<std::string> PlaybackQueue::peek() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (queue_.empty()) {
        return std::nullopt;
    }
    return queue_.front();
}

bool PlaybackQueue::remove(const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = std::find(queue_.begin(), queue_.end(), track_id);
    if (it != queue_.end()) {
        queue_.erase(it);
        return true;
    }
    return false;
}

void PlaybackQueue::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

std::vector<Track> PlaybackQueue::get_queued_tracks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Track> tracks;
    for (const auto& tid : queue_) {
        auto t = library_.get_track_by_id(tid);
        if (t) {
            tracks.push_back(*t);
        }
    }
    return tracks;
}

std::vector<std::string> PlaybackQueue::get_track_ids() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return std::vector<std::string>(queue_.begin(), queue_.end());
}

size_t PlaybackQueue::size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
}

bool PlaybackQueue::empty() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void PlaybackQueue::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.clear();
}

} // namespace tunebox
