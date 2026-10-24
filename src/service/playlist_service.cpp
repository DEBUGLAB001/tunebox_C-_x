#include "service/playlist_service.hpp"
#include <algorithm>

namespace tunebox {

// --- PlaylistTrackList Linked List Implementation ---

void PlaylistTrackList::append(const std::string& track_id) {
    auto new_node = std::make_shared<PlaylistNode>(track_id);
    if (!head_) {
        head_ = new_node;
    } else {
        auto curr = head_;
        while (curr->next) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
    ++size_;
}

bool PlaylistTrackList::remove(const std::string& track_id) {
    if (!head_) return false;

    if (head_->track_id == track_id) {
        head_ = head_->next;
        --size_;
        return true;
    }

    auto curr = head_;
    while (curr->next && curr->next->track_id != track_id) {
        curr = curr->next;
    }

    if (curr->next) {
        curr->next = curr->next->next;
        --size_;
        return true;
    }
    return false;
}

bool PlaylistTrackList::reorder(size_t from_idx, size_t to_idx) {
    if (from_idx >= size_ || to_idx >= size_ || from_idx == to_idx || size_ <= 1) {
        return false;
    }

    // 1. Detach node from from_idx
    std::shared_ptr<PlaylistNode> node_to_move = nullptr;
    if (from_idx == 0) {
        node_to_move = head_;
        head_ = head_->next;
    } else {
        auto prev = head_;
        for (size_t i = 0; i < from_idx - 1 && prev; ++i) {
            prev = prev->next;
        }
        if (!prev || !prev->next) return false;
        node_to_move = prev->next;
        prev->next = node_to_move->next;
    }
    node_to_move->next = nullptr;

    // 2. Insert node at to_idx
    if (to_idx == 0) {
        node_to_move->next = head_;
        head_ = node_to_move;
        return true;
    }

    auto curr = head_;
#ifdef SEED_BUGS
    // Bug 3: Linked list traversal defect - off-by-one traversal error during node reinsertion
    // Loops with <= instead of <, advancing one node too far past the intended destination slot
    for (size_t i = 0; i <= to_idx - 1 && curr && curr->next; ++i) {
        curr = curr->next;
    }
#else
    for (size_t i = 0; i < to_idx - 1 && curr && curr->next; ++i) {
        curr = curr->next;
    }
#endif

    if (!curr) {
        // Fallback safety: append if traversal reached end
        if (!head_) {
            head_ = node_to_move;
        } else {
            auto end = head_;
            while (end->next) end = end->next;
            end->next = node_to_move;
        }
    } else {
        node_to_move->next = curr->next;
        curr->next = node_to_move;
    }

    return true;
}

bool PlaylistTrackList::contains(const std::string& track_id) const {
    auto curr = head_;
    while (curr) {
        if (curr->track_id == track_id) return true;
        curr = curr->next;
    }
    return false;
}

std::vector<std::string> PlaylistTrackList::to_vector() const {
    std::vector<std::string> result;
    auto curr = head_;
    while (curr) {
        result.push_back(curr->track_id);
        curr = curr->next;
    }
    return result;
}

size_t PlaylistTrackList::size() const {
    return size_;
}

bool PlaylistTrackList::empty() const {
    return size_ == 0;
}

void PlaylistTrackList::clear() {
    head_ = nullptr;
    size_ = 0;
}

// --- PlaylistService Implementation ---

PlaylistService::PlaylistService(const LibraryService& library)
    : library_(library) {
    init_seed_playlists();
}

void PlaylistService::init_seed_playlists() {
    playlists_.clear();
    playlist_tracks_.clear();

    // Seed Playlist 1: Neon Highway
    {
        Playlist p1{"pl_01", "Neon Highway", "High-energy synthwave and retro electro for late-night drives", "linear-gradient(135deg, #8b5cf6, #d946ef)", {}, 0, 0};
        PlaylistTrackList list1;
        list1.append("track_01");
        list1.append("track_07");
        list1.append("track_09");
        list1.append("track_16");
        p1.track_ids = list1.to_vector();
        p1.track_count = list1.size();
        p1.total_duration = calculate_total_duration(p1.track_ids);
        playlists_["pl_01"] = p1;
        playlist_tracks_["pl_01"] = std::move(list1);
    }

    // Seed Playlist 2: Midnight Coffee
    {
        Playlist p2{"pl_02", "Midnight Coffee", "Smooth lo-fi chill beats and warm jazz-hop melodies", "linear-gradient(135deg, #f43f5e, #fb923c)", {}, 0, 0};
        PlaylistTrackList list2;
        list2.append("track_02");
        list2.append("track_06");
        list2.append("track_08");
        list2.append("track_12");
        p2.track_ids = list2.to_vector();
        p2.track_count = list2.size();
        p2.total_duration = calculate_total_duration(p2.track_ids);
        playlists_["pl_02"] = p2;
        playlist_tracks_["pl_02"] = std::move(list2);
    }

    // Seed Playlist 3: Deep Focus Matrix
    {
        Playlist p3{"pl_03", "Deep Focus Matrix", "Ambient soundscapes and subtle electronic pulses for creative flow", "linear-gradient(135deg, #06b6d4, #10b981)", {}, 0, 0};
        PlaylistTrackList list3;
        list3.append("track_03");
        list3.append("track_04");
        list3.append("track_10");
        list3.append("track_13");
        list3.append("track_14");
        p3.track_ids = list3.to_vector();
        p3.track_count = list3.size();
        p3.total_duration = calculate_total_duration(p3.track_ids);
        playlists_["pl_03"] = p3;
        playlist_tracks_["pl_03"] = std::move(list3);
    }
}

int PlaylistService::calculate_total_duration(const std::vector<std::string>& track_ids) const {
    int total = 0;
    for (const auto& tid : track_ids) {
        auto track = library_.get_track_by_id(tid);
        if (track) {
            total += track->duration;
        }
    }
    return total;
}

std::vector<Playlist> PlaylistService::get_all_playlists() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Playlist> result;
    for (const auto& pair : playlists_) {
        result.push_back(pair.second);
    }
    std::sort(result.begin(), result.end(), [](const Playlist& a, const Playlist& b) {
        return a.id < b.id;
    });
    return result;
}

std::optional<Playlist> PlaylistService::get_playlist_by_id(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    if (it != playlists_.end()) {
        return it->second;
    }
    return std::nullopt;
}

Playlist PlaylistService::create_playlist(const std::string& title, const std::string& description, const std::string& cover_gradient) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string id = "pl_" + std::to_string(playlists_.size() + 1);
    while (playlists_.find(id) != playlists_.end()) {
        id += "_x";
    }

    std::string gradient = cover_gradient.empty() ? "linear-gradient(135deg, #8b5cf6, #3b82f6)" : cover_gradient;
    Playlist p{id, title, description, gradient, {}, 0, 0};
    playlists_[id] = p;
    playlist_tracks_[id] = PlaylistTrackList();
    return p;
}

bool PlaylistService::update_playlist(const std::string& id, const std::string& title, const std::string& description) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    if (it == playlists_.end()) return false;
    it->second.title = title;
    it->second.description = description;
    return true;
}

bool PlaylistService::delete_playlist(const std::string& id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = playlists_.find(id);
    if (it == playlists_.end()) return false;
    playlists_.erase(it);
    playlist_tracks_.erase(id);
    return true;
}

bool PlaylistService::add_track_to_playlist(const std::string& playlist_id, const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pl_it = playlists_.find(playlist_id);
    if (pl_it == playlists_.end()) return false;

    // Verify track exists in library
    if (!library_.get_track_by_id(track_id)) return false;

    auto& track_list = playlist_tracks_[playlist_id];
    track_list.append(track_id);

    pl_it->second.track_ids = track_list.to_vector();
    pl_it->second.track_count = track_list.size();
    pl_it->second.total_duration = calculate_total_duration(pl_it->second.track_ids);
    return true;
}

bool PlaylistService::remove_track_from_playlist(const std::string& playlist_id, const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pl_it = playlists_.find(playlist_id);
    if (pl_it == playlists_.end()) return false;

    auto& track_list = playlist_tracks_[playlist_id];
    if (!track_list.remove(track_id)) return false;

    pl_it->second.track_ids = track_list.to_vector();
    pl_it->second.track_count = track_list.size();
    pl_it->second.total_duration = calculate_total_duration(pl_it->second.track_ids);
    return true;
}

bool PlaylistService::reorder_playlist_tracks(const std::string& playlist_id, size_t from_idx, size_t to_idx) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pl_it = playlists_.find(playlist_id);
    if (pl_it == playlists_.end()) return false;

    auto& track_list = playlist_tracks_[playlist_id];
    if (!track_list.reorder(from_idx, to_idx)) return false;

    pl_it->second.track_ids = track_list.to_vector();
    return true;
}

std::vector<Track> PlaylistService::get_playlist_tracks(const std::string& playlist_id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto pl_it = playlists_.find(playlist_id);
    if (pl_it == playlists_.end()) return {};

    std::vector<Track> tracks;
    for (const auto& tid : pl_it->second.track_ids) {
        auto track = library_.get_track_by_id(tid);
        if (track) {
            tracks.push_back(*track);
        }
    }
    return tracks;
}

void PlaylistService::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    init_seed_playlists();
}

} // namespace tunebox
