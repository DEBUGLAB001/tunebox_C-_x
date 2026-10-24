// tests/test_suite.cpp
#include "json.hpp"
#include "service/library_service.hpp"
#include "service/search_service.hpp"
#include "service/playlist_service.hpp"
#include "service/playback_queue.hpp"
#include "service/playback_controller.hpp"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>
#include <functional>

using json = nlohmann::json;

struct TestCase {
    std::string name;
    std::function<void()> test_fn;
};

int main() {
    std::vector<TestCase> test_cases;

    // --- TEST 1: Library Array Boundary Behavior (Targeted: Array Indexing Bug) ---
    test_cases.push_back({"test_library_array_boundary", []() {
        tunebox::LibraryService lib;
        auto tracks = lib.get_all_tracks();
        size_t count = lib.get_track_count();

        if (count != 16) {
            throw std::runtime_error("Library total track count expected 16, got " + std::to_string(count));
        }
        if (tracks.size() != 16) {
            throw std::runtime_error("Array indexing off-by-one: get_all_tracks() returned " + std::to_string(tracks.size()) + " instead of 16 tracks (last track skipped)");
        }
        auto last_track = lib.get_track_at_index(15);
        if (!last_track || last_track->id != "track_16") {
            throw std::runtime_error("Boundary index 15 failed to return valid last track track_16");
        }
    }});

    // --- TEST 2: Search String Processing (Targeted: String Processing Bug) ---
    test_cases.push_back({"test_search_string_processing", []() {
        tunebox::LibraryService lib;
        tunebox::SearchService search(lib);

        auto res_ws = search.search("  Synthwave  ");
        if (res_ws.empty()) {
            throw std::runtime_error("String processing defect: query with leading/trailing whitespace failed to match tracks");
        }
        auto res_case = search.search("cyberpunk");
        auto res_case_upper = search.search("CYBERPUNK");
        if (res_case.empty() || res_case_upper.empty() || res_case.size() != res_case_upper.size()) {
            throw std::runtime_error("String processing defect: case-insensitive matching failed for query");
        }
    }});

    // --- TEST 3: Playlist Linked List Traversal & Reordering (Targeted: Linked List Bug) ---
    test_cases.push_back({"test_playlist_linked_list_traversal", []() {
        tunebox::LibraryService lib;
        tunebox::PlaylistService pl_svc(lib);

        auto p = pl_svc.create_playlist("Test Playlist", "Testing linked list order");
        pl_svc.add_track_to_playlist(p.id, "track_01");
        pl_svc.add_track_to_playlist(p.id, "track_02");
        pl_svc.add_track_to_playlist(p.id, "track_03");
        pl_svc.add_track_to_playlist(p.id, "track_04");

        bool ok = pl_svc.reorder_playlist_tracks(p.id, 0, 2);
        if (!ok) {
            throw std::runtime_error("Playlist reorder operation returned false");
        }
        auto pl_after = pl_svc.get_playlist_by_id(p.id);
        if (!pl_after || pl_after->track_ids.size() != 4) {
            throw std::runtime_error("Playlist track count invalid after reorder");
        }
        const auto& ids = pl_after->track_ids;
        if (ids[0] != "track_02" || ids[1] != "track_03" || ids[2] != "track_01" || ids[3] != "track_04") {
            throw std::runtime_error("Linked list traversal defect: reorder placed node at incorrect position [" +
                                     ids[0] + ", " + ids[1] + ", " + ids[2] + ", " + ids[3] + "]");
        }
    }});

    // --- TEST 4: FIFO Playback Queue Behavior (Targeted: FIFO Queue Bug) ---
    test_cases.push_back({"test_playback_queue_fifo", []() {
        tunebox::LibraryService lib;
        tunebox::PlaybackQueue q(lib);

        q.enqueue("track_01");
        q.enqueue("track_02");
        q.enqueue("track_03");

        if (q.size() != 3) {
            throw std::runtime_error("Queue size expected 3 after 3 enqueues, got " + std::to_string(q.size()));
        }
        std::string first = q.dequeue();
        if (first != "track_01") {
            throw std::runtime_error("FIFO queue defect: first dequeue returned '" + first + "' instead of 'track_01' (skipped or wrong track)");
        }
        std::string second = q.dequeue();
        if (second != "track_02") {
            throw std::runtime_error("FIFO queue defect: second dequeue returned '" + second + "' instead of 'track_02'");
        }
        if (q.size() != 1) {
            throw std::runtime_error("Queue size mismatch after dequeues");
        }
    }});

    // --- RUN & AGGREGATE RESULTS ---
    json report;
    int passed = 0;
    int failed = 0;
    auto suite_start = std::chrono::steady_clock::now();

    for (const auto& tc : test_cases) {
        auto t_start = std::chrono::steady_clock::now();
        try {
            tc.test_fn();
            auto t_end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
            report[tc.name] = {
                {"Status", "passed"},
                {"Execution time", std::to_string(elapsed) + "ms"}
            };
            passed++;
        } catch (const std::exception& ex) {
            auto t_end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
            report[tc.name] = {
                {"Status", "failed"},
                {"Execution time", std::to_string(elapsed) + "ms"},
                {"Error", ex.what()}
            };
            failed++;
        }
    }

    auto suite_end = std::chrono::steady_clock::now();
    auto total_ms = std::chrono::duration_cast<std::chrono::milliseconds>(suite_end - suite_start).count();

    report["Passed"] = passed;
    report["Failed"] = failed;
    report["Total bugs"] = static_cast<int>(test_cases.size());
    report["total"] = static_cast<int>(test_cases.size());
    report["passed"] = passed;
    report["failed"] = failed;
    report["Total Execution time"] = std::to_string(total_ms) + "ms";

    // Output pure JSON to stdout
    std::cout << report.dump(2) << std::endl;

    // Return non-zero if any test failed
    return (failed == 0) ? 0 : 1;
}
