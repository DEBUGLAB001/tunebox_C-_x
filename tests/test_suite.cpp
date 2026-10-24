#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include "json.hpp"
#include "service/library_service.hpp"
#include "service/search_service.hpp"
#include "service/playlist_service.hpp"
#include "service/playback_queue.hpp"
#include "service/playback_controller.hpp"

using json = nlohmann::json;

struct TestCaseResult {
    std::string name;
    std::string status; // "PASSED" or "FAILED"
    std::string message;
};

int main() {
    std::vector<TestCaseResult> results;
    auto start_time = std::chrono::high_resolution_clock::now();

    // --- Test 1: Library Array Boundary Behavior (Targeted: Array Indexing Bug) ---
    {
        TestCaseResult tr{"test_library_array_boundary", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            auto tracks = lib.get_all_tracks();
            size_t count = lib.get_track_count();

            // Total seeded tracks must be 16
            if (count != 16) {
                tr.status = "FAILED";
                tr.message = "Library total track count expected 16, got " + std::to_string(count);
            } else if (tracks.size() != 16) {
                tr.status = "FAILED";
                tr.message = "Array indexing off-by-one: get_all_tracks() returned " + std::to_string(tracks.size()) + " instead of 16 tracks (last track skipped)";
            } else {
                // Verify last boundary track
                auto last_track = lib.get_track_at_index(15);
                if (!last_track || last_track->id != "track_16") {
                    tr.status = "FAILED";
                    tr.message = "Boundary index 15 failed to return valid last track track_16";
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 2: Search String Processing (Targeted: String Processing Bug) ---
    {
        TestCaseResult tr{"test_search_string_processing", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::SearchService search(lib);

            // 1. Whitespace handling
            auto res_ws = search.search("  Synthwave  ");
            if (res_ws.empty()) {
                tr.status = "FAILED";
                tr.message = "String processing defect: query with leading/trailing whitespace failed to match tracks";
            } else {
                // 2. Case-insensitive matching
                auto res_case = search.search("cyberpunk");
                auto res_case_upper = search.search("CYBERPUNK");
                if (res_case.empty() || res_case_upper.empty() || res_case.size() != res_case_upper.size()) {
                    tr.status = "FAILED";
                    tr.message = "String processing defect: case-insensitive matching failed for query";
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 3: Playlist Linked List Traversal & Reordering (Targeted: Linked List Bug) ---
    {
        TestCaseResult tr{"test_playlist_linked_list_traversal", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::PlaylistService pl_svc(lib);

            auto p = pl_svc.create_playlist("Test Playlist", "Testing linked list order");
            pl_svc.add_track_to_playlist(p.id, "track_01");
            pl_svc.add_track_to_playlist(p.id, "track_02");
            pl_svc.add_track_to_playlist(p.id, "track_03");
            pl_svc.add_track_to_playlist(p.id, "track_04");

            // Reorder track at index 0 (track_01) to index 2
            // Expected list order: track_02, track_03, track_01, track_04
            bool ok = pl_svc.reorder_playlist_tracks(p.id, 0, 2);
            if (!ok) {
                tr.status = "FAILED";
                tr.message = "Playlist reorder operation returned false";
            } else {
                auto pl_after = pl_svc.get_playlist_by_id(p.id);
                if (!pl_after || pl_after->track_ids.size() != 4) {
                    tr.status = "FAILED";
                    tr.message = "Playlist track count invalid after reorder";
                } else {
                    const auto& ids = pl_after->track_ids;
                    if (ids[0] != "track_02" || ids[1] != "track_03" || ids[2] != "track_01" || ids[3] != "track_04") {
                        tr.status = "FAILED";
                        tr.message = "Linked list traversal defect: reorder placed node at incorrect position [" +
                                     ids[0] + ", " + ids[1] + ", " + ids[2] + ", " + ids[3] + "]";
                    }
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 4: FIFO Playback Queue Behavior (Targeted: FIFO Queue Bug) ---
    {
        TestCaseResult tr{"test_playback_queue_fifo", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::PlaybackQueue q(lib);

            q.enqueue("track_01");
            q.enqueue("track_02");
            q.enqueue("track_03");

            if (q.size() != 3) {
                tr.status = "FAILED";
                tr.message = "Queue size expected 3 after 3 enqueues, got " + std::to_string(q.size());
            } else {
                // First dequeue must return track_01
                std::string first = q.dequeue();
                if (first != "track_01") {
                    tr.status = "FAILED";
                    tr.message = "FIFO queue defect: first dequeue returned '" + first + "' instead of 'track_01' (skipped or wrong track)";
                } else {
                    // Second dequeue must return track_02
                    std::string second = q.dequeue();
                    if (second != "track_02") {
                        tr.status = "FAILED";
                        tr.message = "FIFO queue defect: second dequeue returned '" + second + "' instead of 'track_02'";
                    } else if (q.size() != 1) {
                        tr.status = "FAILED";
                        tr.message = "Queue size mismatch after dequeues";
                    }
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 5: Playback Controller Flow (Regression) ---
    {
        TestCaseResult tr{"test_playback_controller_flow", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::PlaybackQueue q(lib);
            tunebox::PlaybackController ctrl(lib, q);

            ctrl.play("track_02");
            auto st = ctrl.get_state();
            if (!st.is_playing || st.current_track_id != "track_02") {
                tr.status = "FAILED";
                tr.message = "Player failed to start track_02";
            } else {
                ctrl.pause();
                st = ctrl.get_state();
                if (st.is_playing) {
                    tr.status = "FAILED";
                    tr.message = "Player failed to pause";
                } else {
                    ctrl.seek(45);
                    ctrl.set_volume(90);
                    st = ctrl.get_state();
                    if (st.position_sec != 45 || st.volume != 90) {
                        tr.status = "FAILED";
                        tr.message = "Seek or volume failed to update state";
                    }
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 6: Playlist and Library Consistency (Regression) ---
    {
        TestCaseResult tr{"test_playlist_and_library_consistency", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::PlaylistService pl_svc(lib);

            auto playlists = pl_svc.get_all_playlists();
            if (playlists.empty()) {
                tr.status = "FAILED";
                tr.message = "Seed playlists should not be empty";
            } else {
                for (const auto& pl : playlists) {
                    if (pl.track_ids.size() != pl.track_count) {
                        tr.status = "FAILED";
                        tr.message = "Track count mismatch in playlist " + pl.id;
                        break;
                    }
                    if (pl.total_duration <= 0 && pl.track_count > 0) {
                        tr.status = "FAILED";
                        tr.message = "Total duration invalid for playlist " + pl.id;
                        break;
                    }
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 7: Empty Collections and Boundaries (Regression) ---
    {
        TestCaseResult tr{"test_empty_collections_and_boundaries", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::SearchService search(lib);
            tunebox::PlaybackQueue q(lib);

            // Empty query returns empty
            auto empty_res = search.search("");
            if (!empty_res.empty()) {
                tr.status = "FAILED";
                tr.message = "Empty search query should return empty results";
            } else {
                // Empty queue dequeue returns empty string safely
                q.clear();
                std::string popped = q.dequeue();
                if (!popped.empty()) {
                    tr.status = "FAILED";
                    tr.message = "Empty queue dequeue should return empty string";
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    // --- Test 8: Liked Songs and History (Regression) ---
    {
        TestCaseResult tr{"test_liked_songs_and_history", "PASSED", ""};
        try {
            tunebox::LibraryService lib;
            tunebox::PlaybackQueue q(lib);
            tunebox::PlaybackController ctrl(lib, q);

            bool liked = lib.toggle_like("track_05");
            if (!liked) {
                tr.status = "FAILED";
                tr.message = "toggle_like failed to like track_05";
            } else {
                auto liked_list = lib.get_liked_tracks();
                bool found = false;
                for (const auto& t : liked_list) {
                    if (t.id == "track_05") { found = true; break; }
                }
                if (!found) {
                    tr.status = "FAILED";
                    tr.message = "Liked songs collection did not contain track_05";
                } else {
                    ctrl.play("track_05");
                    ctrl.play("track_07");
                    auto history = ctrl.get_history();
                    if (history.empty() || history.front().id != "track_07") {
                        tr.status = "FAILED";
                        tr.message = "History did not record most recent track at front";
                    }
                }
            }
        } catch (const std::exception& e) {
            tr.status = "FAILED";
            tr.message = std::string("Exception: ") + e.what();
        }
        results.push_back(tr);
    }

    auto end_time = std::chrono::high_resolution_clock::now();
    long long duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    int passed = 0;
    int failed = 0;
    json tests_json = json::array();

    for (const auto& r : results) {
        if (r.status == "PASSED") {
            passed++;
        } else {
            failed++;
        }
        tests_json.push_back({
            {"name", r.name},
            {"status", r.status},
            {"message", r.message}
        });
    }

    json report = {
        {"total", results.size()},
        {"passed", passed},
        {"failed", failed},
        {"duration_ms", duration_ms},
        {"tests", tests_json}
    };

    // Diagnostics to stderr
    std::cerr << "[TEST HARNESS] Total: " << results.size() << ", Passed: " << passed << ", Failed: " << failed << std::endl;
    for (const auto& r : results) {
        if (r.status == "FAILED") {
            std::cerr << "  FAIL: " << r.name << " - " << r.message << std::endl;
        }
    }

    // Pure JSON report to stdout
    std::cout << report.dump(2) << std::endl;

    return (failed == 0) ? 0 : 1;
}
