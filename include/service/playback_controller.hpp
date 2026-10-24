#pragma once

#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <optional>
#include "model/music_models.hpp"
#include "service/library_service.hpp"
#include "service/playback_queue.hpp"

namespace tunebox {

class PlaybackController {
public:
    PlaybackController(LibraryService& library, PlaybackQueue& queue);

    // Play specific track or resume current
    bool play(const std::string& track_id = "");

    // Pause playback
    void pause();

    // Advance to next track (from FIFO queue if available, else next in library)
    bool next();

    // Go back to previous track in history
    bool previous();

    // Seek to specific position in seconds
    bool seek(int position_sec);

    // Set volume level (0 - 100)
    void set_volume(int volume);

    // Set repeat mode: "off", "all", "one"
    void set_repeat_mode(const std::string& mode);

    // Toggle shuffle mode
    void toggle_shuffle();

    // Get current playback state with populated track metadata
    PlaybackState get_state() const;

    // Get recently played listening history
    std::vector<Track> get_history() const;

    // Reset playback controller
    void reset();

private:
    void record_history(const std::string& track_id);

    mutable std::mutex mutex_;
    LibraryService& library_;
    PlaybackQueue& queue_;

    PlaybackState state_;
    std::deque<std::string> history_; // most recent first
    static constexpr size_t MAX_HISTORY = 30;
};

} // namespace tunebox
