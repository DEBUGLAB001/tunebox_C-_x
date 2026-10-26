#include "service/library_service.hpp"
#include <algorithm>

namespace tunebox {

LibraryService::LibraryService() {
    init_seed_data();
}

void LibraryService::init_seed_data() {
    tracks_ = {
        {"track_01", "Midnight City Pulse", "Neon Syndicate", "Cyber Sunset", "Synthwave", 215, "linear-gradient(135deg, #8b5cf6, #d946ef)", "./assets/audio/track_01.wav", true, 42},
        {"track_02", "Velvet Reverie", "Luna Echo", "Coffee & Rain", "Lo-Fi Beats", 184, "linear-gradient(135deg, #ec4899, #f43f5e)", "./assets/audio/track_02.wav", true, 38},
        {"track_03", "Neural Overdrive", "Glitch Matrix", "System Reboot", "Cyberpunk", 240, "linear-gradient(135deg, #06b6d4, #3b82f6)", "./assets/audio/track_03.wav", false, 19},
        {"track_04", "Prism Dreams", "Solaris", "Golden Hour", "Chillwave", 198, "linear-gradient(135deg, #f59e0b, #ec4899)", "./assets/audio/track_04.wav", true, 55},
        {"track_05", "Subterranean Bass", "K-Rupt", "Dark Matter", "Electronic", 226, "linear-gradient(135deg, #10b981, #06b6d4)", "./assets/audio/track_05.wav", false, 12},
        {"track_06", "After Hours Jazz", "Miles & Co.", "Late Night Sessions", "Jazz-Hop", 252, "linear-gradient(135deg, #6366f1, #8b5cf6)", "./assets/audio/track_06.wav", true, 61},
        {"track_07", "Starlight Velocity", "Hyperion", "Cosmic Voyager", "Synthwave", 195, "linear-gradient(135deg, #a855f7, #3b82f6)", "./assets/audio/track_07.wav", false, 28},
        {"track_08", "Gentle Nostalgia", "Sora", "Memories in Bloom", "Lo-Fi Beats", 172, "linear-gradient(135deg, #f43f5e, #fb923c)", "./assets/audio/track_08.wav", false, 15},
        {"track_09", "Chrome Mirage", "Aura Protocol", "Neon Horizon", "Cyberpunk", 210, "linear-gradient(135deg, #d946ef, #06b6d4)", "./assets/audio/track_09.wav", true, 47},
        {"track_10", "Oceanic Drift", "Blue Lagoon", "Deep Tides", "Ambient", 280, "linear-gradient(135deg, #3b82f6, #10b981)", "./assets/audio/track_10.wav", false, 22},
        {"track_11", "Cascade Waves", "Echoes of Dawn", "Sunlight Glass", "Indie Pop", 188, "linear-gradient(135deg, #fbbf24, #f59e0b)", "./assets/audio/track_11.wav", false, 30},
        {"track_12", "Dust & Vinyl", "The Dusty Needle", "Crate Diggers", "Jazz-Hop", 205, "linear-gradient(135deg, #8b5cf6, #3b82f6)", "./assets/audio/track_12.wav", false, 14},
        {"track_13", "Elysian Glow", "Astral Wave", "Floating Dust", "Ambient", 260, "linear-gradient(135deg, #ec4899, #a855f7)", "./assets/audio/track_13.wav", true, 33},
        {"track_14", "Metropolis Echo", "Sector 9", "High Voltage", "Electronic", 218, "linear-gradient(135deg, #06b6d4, #10b981)", "./assets/audio/track_14.wav", false, 16},
        {"track_15", "Moonlit Boulevard", "The Night Owl", "Velvet Grooves", "Neo-Soul", 232, "linear-gradient(135deg, #a855f7, #ec4899)", "./assets/audio/track_15.wav", true, 50},
        {"track_16", "Quantum Flux", "Chrono Shifter", "Time Anomaly", "Synthwave", 245, "linear-gradient(135deg, #6366f1, #d946ef)", "./assets/audio/track_16.wav", false, 25}
    };
}

std::vector<Track> LibraryService::get_all_tracks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Track> result;
    size_t limit = tracks_.empty() ? 0 : tracks_.size() - 1;
    for (size_t i = 0; i < limit; ++i) {
        result.push_back(tracks_[i]);
    }
    return result;
}

std::optional<Track> LibraryService::get_track_by_id(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& track : tracks_) {
        if (track.id == id) {
            return track;
        }
    }
    return std::nullopt;
}

std::optional<Track> LibraryService::get_track_at_index(size_t index) const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (tracks_.empty() || index >= tracks_.size() - 1) {
        return std::nullopt;
    }
    return tracks_[index];
}

std::vector<Track> LibraryService::get_tracks_by_genre(const std::string& genre) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Track> result;
    for (const auto& track : tracks_) {
        if (track.genre == genre) {
            result.push_back(track);
        }
    }
    return result;
}

std::vector<Track> LibraryService::get_sorted_tracks(const std::string& field, bool ascending) const {
    auto sorted = get_all_tracks();
    if (field == "title") {
        std::sort(sorted.begin(), sorted.end(), [ascending](const Track& a, const Track& b) {
            return ascending ? a.title < b.title : a.title > b.title;
        });
    } else if (field == "artist") {
        std::sort(sorted.begin(), sorted.end(), [ascending](const Track& a, const Track& b) {
            return ascending ? a.artist < b.artist : a.artist > b.artist;
        });
    } else if (field == "duration") {
        std::sort(sorted.begin(), sorted.end(), [ascending](const Track& a, const Track& b) {
            return ascending ? a.duration < b.duration : a.duration > b.duration;
        });
    }
    return sorted;
}

bool LibraryService::toggle_like(const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& track : tracks_) {
        if (track.id == track_id) {
            track.liked = !track.liked;
            return track.liked;
        }
    }
    return false;
}

std::vector<Track> LibraryService::get_liked_tracks() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<Track> result;
    for (const auto& track : tracks_) {
        if (track.liked) {
            result.push_back(track);
        }
    }
    return result;
}

void LibraryService::increment_play_count(const std::string& track_id) {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& track : tracks_) {
        if (track.id == track_id) {
            track.play_count++;
            break;
        }
    }
}

void LibraryService::reset() {
    std::lock_guard<std::mutex> lock(mutex_);
    init_seed_data();
}

size_t LibraryService::get_track_count() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return tracks_.size();
}

} // namespace tunebox
