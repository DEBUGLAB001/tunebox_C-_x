#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_map>
#include <optional>
#include <mutex>
#include "model/music_models.hpp"
#include "service/library_service.hpp"

namespace tunebox {

// Linked list node for ordered playlist tracks
struct PlaylistNode {
    std::string track_id;
    std::shared_ptr<PlaylistNode> next;

    explicit PlaylistNode(std::string id) : track_id(std::move(id)), next(nullptr) {}
};

// Linked list maintaining playlist track order
class PlaylistTrackList {
public:
    PlaylistTrackList() = default;

    void append(const std::string& track_id);
    bool remove(const std::string& track_id);
    bool reorder(size_t from_idx, size_t to_idx);
    bool contains(const std::string& track_id) const;
    std::vector<std::string> to_vector() const;
    size_t size() const;
    bool empty() const;
    void clear();

private:
    std::shared_ptr<PlaylistNode> head_ = nullptr;
    size_t size_ = 0;
};

class PlaylistService {
public:
    explicit PlaylistService(const LibraryService& library);

    // List all playlists
    std::vector<Playlist> get_all_playlists() const;

    // Get single playlist by ID
    std::optional<Playlist> get_playlist_by_id(const std::string& id) const;

    // Create a new playlist
    Playlist create_playlist(const std::string& title, const std::string& description, const std::string& cover_gradient = "");

    // Update playlist metadata
    bool update_playlist(const std::string& id, const std::string& title, const std::string& description);

    // Delete playlist
    bool delete_playlist(const std::string& id);

    // Add track to playlist (appends to linked list)
    bool add_track_to_playlist(const std::string& playlist_id, const std::string& track_id);

    // Remove track from playlist (removes from linked list)
    bool remove_track_from_playlist(const std::string& playlist_id, const std::string& track_id);

    // Reorder tracks within playlist using linked list operations
    bool reorder_playlist_tracks(const std::string& playlist_id, size_t from_idx, size_t to_idx);

    // Get populated Track objects for a playlist in linked list order
    std::vector<Track> get_playlist_tracks(const std::string& playlist_id) const;

    // Reset playlists to default seeds
    void reset();

private:
    void init_seed_playlists();
    int calculate_total_duration(const std::vector<std::string>& track_ids) const;

    mutable std::mutex mutex_;
    const LibraryService& library_;
    std::unordered_map<std::string, Playlist> playlists_;
    std::unordered_map<std::string, PlaylistTrackList> playlist_tracks_;
};

} // namespace tunebox
