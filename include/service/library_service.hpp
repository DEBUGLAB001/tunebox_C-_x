#pragma once

#include <vector>
#include <string>
#include <optional>
#include <mutex>
#include "model/music_models.hpp"

namespace tunebox {

class LibraryService {
public:
    LibraryService();

    // Get all tracks in the library
    std::vector<Track> get_all_tracks() const;

    // Get a track by ID
    std::optional<Track> get_track_by_id(const std::string& id) const;

    // Get track at boundary index
    std::optional<Track> get_track_at_index(size_t index) const;

    // Get tracks filtered by genre
    std::vector<Track> get_tracks_by_genre(const std::string& genre) const;

    // Sort tracks by field (title, artist, duration)
    std::vector<Track> get_sorted_tracks(const std::string& field, bool ascending = true) const;

    // Like / unlike a track
    bool toggle_like(const std::string& track_id);

    // Get all liked tracks
    std::vector<Track> get_liked_tracks() const;

    // Increment play count
    void increment_play_count(const std::string& track_id);

    // Reset library to initial seed data
    void reset();

    // Total track count
    size_t get_track_count() const;

private:
    void init_seed_data();

    mutable std::mutex mutex_;
    std::vector<Track> tracks_;
};

} // namespace tunebox
