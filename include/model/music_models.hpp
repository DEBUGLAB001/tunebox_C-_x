#pragma once

#include <string>
#include <vector>
#include "json.hpp"

namespace tunebox {

struct Track {
    std::string id;
    std::string title;
    std::string artist;
    std::string album;
    std::string genre;
    int duration = 0; // seconds
    std::string artwork; // CSS gradient or image URL
    std::string audio_url; // Audio asset URL
    bool liked = false;
    int play_count = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Track, id, title, artist, album, genre, duration, artwork, audio_url, liked, play_count)
};

struct Playlist {
    std::string id;
    std::string title;
    std::string description;
    std::string cover_gradient;
    std::vector<std::string> track_ids;
    int total_duration = 0;
    size_t track_count = 0;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Playlist, id, title, description, cover_gradient, track_ids, total_duration, track_count)
};

struct PlaybackState {
    std::string current_track_id;
    bool is_playing = false;
    int position_sec = 0;
    int volume = 80;
    std::string repeat_mode = "off"; // "off", "all", "one"
    bool shuffle = false;
    nlohmann::json current_track = nullptr;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(PlaybackState, current_track_id, is_playing, position_sec, volume, repeat_mode, shuffle, current_track)
};

} // namespace tunebox
