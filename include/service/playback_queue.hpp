#pragma once

#include <deque>
#include <vector>
#include <string>
#include <optional>
#include <mutex>
#include "model/music_models.hpp"
#include "service/library_service.hpp"

namespace tunebox {

class PlaybackQueue {
public:
    explicit PlaybackQueue(const LibraryService& library);

    // Enqueue track to the back of the FIFO queue
    void enqueue(const std::string& track_id);

    // Pop the next track from the front of the FIFO queue
    std::string dequeue();

    // Inspect the next track at the front of the FIFO queue without removing
    std::optional<std::string> peek() const;

    // Remove a specific track ID from the queue
    bool remove(const std::string& track_id);

    // Clear entire queue
    void clear();

    // Get all queued tracks in FIFO order with full track details
    std::vector<Track> get_queued_tracks() const;

    // Get list of queued track IDs in order
    std::vector<std::string> get_track_ids() const;

    // Queue size
    size_t size() const;

    // Check if queue is empty
    bool empty() const;

    // Reset queue
    void reset();

private:
    mutable std::mutex mutex_;
    const LibraryService& library_;
    std::deque<std::string> queue_;
};

} // namespace tunebox
