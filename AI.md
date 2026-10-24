# TuneBox

## Description

TuneBox is a desktop-inspired web music player and playlist manager built with C++17 using an embedded `cpp-httplib` HTTP server and `nlohmann/json`. The application bootstrap in `src/main.cpp` serves static frontend assets from `public/` and exposes REST endpoints under `/api/` for library browsing, search, playlist operations, FIFO queue management, and playback control. Core domain logic is encapsulated in `core_services`: `LibraryService` manages an in-memory catalog of 16 tracks with like states and play counts; `SearchService` executes multi-field queries across title, artist, album, and genre; `PlaylistService` maintains playlist sequences via a singly linked list (`PlaylistTrackList`) supporting arbitrary node reordering; and `PlaybackQueue` regulates upcoming playback via a FIFO queue consumed by `PlaybackController`.

## Repository Structure

```text
.
├── challenge.json              # Challenge runtime configuration and bug topics
├── CMakeLists.txt              # CMake build definitions (core_services, server)
├── start.sh                    # Startup and incremental hot-reload script
├── include/
│   ├── httplib.h               # Embedded HTTP server library
│   ├── json.hpp                # Embedded JSON serializer
│   ├── model/
│   │   └── music_models.hpp    # Domain data models (Track, Playlist, PlaybackState)
│   └── service/
│       ├── library_service.hpp # Library catalog service interface
│       ├── search_service.hpp  # Track search service interface
│       ├── playlist_service.hpp# Playlist service and linked list interface
│       ├── playback_queue.hpp  # FIFO playback queue interface
│       └── playback_controller.hpp # Playback controller state machine interface
├── src/
│   ├── main.cpp                # HTTP server bootstrap and route definitions
│   └── service/
│       ├── library_service.cpp # Catalog storage, genre filter, and like toggling
│       ├── search_service.cpp  # Case-insensitive multi-field search engine
│       ├── playlist_service.cpp# Playlist management and linked-list reordering
│       ├── playback_queue.cpp  # FIFO playback queue implementation
│       └── playback_controller.cpp # Playback state transitions, history, and volume
└── public/
    ├── index.html              # Single-page desktop-inspired music player layout
    ├── css/
    │   └── style.css           # Visual theme, typography, and responsive styles
    ├── js/
    │   └── app.js              # State synchronization, audio engine, and player controls
    └── assets/
        └── audio/              # Synthesized sample audio tracks (.wav)
```

## Bugs and Bug Locations

1. Array Indexing Boundary Defect

- Bug location: `src/service/library_service.cpp`, within `LibraryService::get_all_tracks()` (lines 35–40) and `LibraryService::get_track_at_index()` (lines 61–65).
- How to observe it: Send `GET /api/tracks` or query the boundary track at index 15 (`track_16`).
- Failure: The loop limit in `get_all_tracks()` terminates early at `tracks_.size() - 1`, and `get_track_at_index()` rejects index `tracks_.size() - 1`. This silently skips the 16th track ("Quantum Flux"), returning only 15 tracks.
- Expected: The iteration must check `i < tracks_.size()`, returning all 16 library tracks and accepting valid boundary index 15.

2. String Processing Normalization Defect

- Bug location: `src/service/search_service.cpp`, within `SearchService::search()` (lines 27–32, 44–48).
- How to observe it: Send `GET /api/search?q=%20Synthwave%20` or `GET /api/search?q=CYBERPUNK`.
- Failure: The search query is not trimmed of leading/trailing whitespace and is compared without lowercasing against track fields, causing queries with extra whitespace or uppercase characters to return 0 results.
- Expected: Queries must be trimmed and converted to lowercase, performing case-insensitive substring matching across title, artist, album, and genre.

3. Linked List Traversal Defect

- Bug location: `src/service/playlist_service.cpp`, within `PlaylistTrackList::reorder()` (lines 74–78).
- How to observe it: Send `POST /api/playlists/pl_01/reorder` with `{"from_index": 0, "to_index": 2}`.
- Failure: The reinsertion traversal loop uses `<=` instead of `<` (`for (size_t i = 0; i <= to_idx - 1 && curr && curr->next; ++i)`), advancing one node too far past the destination slot and inserting the moved node at the wrong position.
- Expected: The loop must advance `to_idx - 1` times (`i < to_idx - 1`), placing the moved node precisely at the specified target index.

4. FIFO Queue Dequeue Defect

- Bug location: `src/service/playback_queue.cpp`, within `PlaybackQueue::dequeue()` (lines 23–30).
- How to observe it: Enqueue two tracks (`track_02` then `track_03`) via `POST /api/queue` and call `POST /api/queue/next`.
- Failure: `dequeue()` calls `queue_.pop_front()` twice, discarding the first queued track and returning the second one, violating FIFO queue order.
- Expected: `dequeue()` must retrieve `queue_.front()`, call `queue_.pop_front()` exactly once, and return the first queued track.
