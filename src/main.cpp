#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#endif

#include "httplib.h"
#include "json.hpp"
#include "service/library_service.hpp"
#include "service/search_service.hpp"
#include "service/playlist_service.hpp"
#include "service/playback_queue.hpp"
#include "service/playback_controller.hpp"

#include <iostream>
#include <cstdlib>
#include <string>

using json = nlohmann::json;

int main(int argc, char* argv[]) {
    tunebox::LibraryService library;
    tunebox::SearchService search(library);
    tunebox::PlaylistService playlists(library);
    tunebox::PlaybackQueue queue(library);
    tunebox::PlaybackController player(library, queue);

    httplib::Server svr;

    // Static files mount
    svr.set_mount_point("/", "./public");
    svr.set_file_extension_and_mimetype_mapping("wav", "audio/wav");
    svr.set_file_extension_and_mimetype_mapping("js", "application/javascript");
    svr.set_file_extension_and_mimetype_mapping("css", "text/css");
    svr.set_file_extension_and_mimetype_mapping("html", "text/html");

    // Global CORS pre-routing
    svr.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
        if (req.method == "OPTIONS") {
            res.status = 204;
            return httplib::Server::HandlerResponse::Handled;
        }
        return httplib::Server::HandlerResponse::Unhandled;
    });

    // --- Health ---
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"status", "ok"},
            {"service", "tunebox"},
            {"version", "1.0.0"}
        };
        res.set_content(body.dump(), "application/json");
    });

    // --- Aggregated App State ---
    svr.Get("/api/state", [&](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"player", player.get_state()},
            {"queue", queue.get_queued_tracks()},
            {"history", player.get_history()},
            {"playlists", playlists.get_all_playlists()},
            {"liked", library.get_liked_tracks()},
            {"track_count", library.get_track_count()}
        };
        res.set_content(body.dump(), "application/json");
    });

    // --- Tracks ---
    svr.Get("/api/tracks", [&](const httplib::Request& req, httplib::Response& res) {
        if (req.has_param("genre")) {
            std::string genre = req.get_param_value("genre");
            json body = library.get_tracks_by_genre(genre);
            res.set_content(body.dump(), "application/json");
        } else if (req.has_param("sort")) {
            std::string sort_field = req.get_param_value("sort");
            bool asc = true;
            if (req.has_param("asc")) {
                asc = (req.get_param_value("asc") != "false");
            }
            json body = library.get_sorted_tracks(sort_field, asc);
            res.set_content(body.dump(), "application/json");
        } else {
            json body = library.get_all_tracks();
            res.set_content(body.dump(), "application/json");
        }
    });

    svr.Get(R"(/api/tracks/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto track = library.get_track_by_id(id);
        if (track) {
            json body = *track;
            res.set_content(body.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(json{{"error", "Track not found"}}.dump(), "application/json");
        }
    });

    svr.Post(R"(/api/tracks/([a-zA-Z0-9_\-]+)/like)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto track = library.get_track_by_id(id);
        if (!track) {
            res.status = 404;
            res.set_content(json{{"error", "Track not found"}}.dump(), "application/json");
            return;
        }
        bool liked = library.toggle_like(id);
        res.set_content(json{{"track_id", id}, {"liked", liked}}.dump(), "application/json");
    });

    // --- Search ---
    svr.Get("/api/search", [&](const httplib::Request& req, httplib::Response& res) {
        std::string q = req.has_param("q") ? req.get_param_value("q") : "";
        std::string field = req.has_param("field") ? req.get_param_value("field") : "all";
        json results = search.search(q, field);
        res.set_content(results.dump(), "application/json");
    });

    // --- Playlists ---
    svr.Get("/api/playlists", [&](const httplib::Request&, httplib::Response& res) {
        json body = playlists.get_all_playlists();
        res.set_content(body.dump(), "application/json");
    });

    svr.Post("/api/playlists", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string title = body.value("title", "My Playlist");
            std::string desc = body.value("description", "");
            std::string gradient = body.value("cover_gradient", "");
            auto created = playlists.create_playlist(title, desc, gradient);
            res.status = 201;
            res.set_content(json(created).dump(), "application/json");
        } catch (const std::exception& e) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON payload"}}.dump(), "application/json");
        }
    });

    svr.Get(R"(/api/playlists/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        auto pl = playlists.get_playlist_by_id(id);
        if (pl) {
            auto tracks = playlists.get_playlist_tracks(id);
            json body = *pl;
            body["tracks"] = tracks;
            res.set_content(body.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(json{{"error", "Playlist not found"}}.dump(), "application/json");
        }
    });

    svr.Put(R"(/api/playlists/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto body = json::parse(req.body);
            std::string title = body.value("title", "");
            std::string desc = body.value("description", "");
            if (playlists.update_playlist(id, title, desc)) {
                res.set_content(json{{"success", true}}.dump(), "application/json");
            } else {
                res.status = 404;
                res.set_content(json{{"error", "Playlist not found"}}.dump(), "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Delete(R"(/api/playlists/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        if (playlists.delete_playlist(id)) {
            res.set_content(json{{"success", true}}.dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(json{{"error", "Playlist not found"}}.dump(), "application/json");
        }
    });

    svr.Post(R"(/api/playlists/([a-zA-Z0-9_\-]+)/tracks)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto body = json::parse(req.body);
            std::string track_id = body.value("track_id", "");
            if (playlists.add_track_to_playlist(id, track_id)) {
                auto pl = playlists.get_playlist_by_id(id);
                res.set_content(json(*pl).dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"error", "Failed to add track to playlist"}}.dump(), "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Delete(R"(/api/playlists/([a-zA-Z0-9_\-]+)/tracks/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        std::string track_id = req.matches[2];
        if (playlists.remove_track_from_playlist(id, track_id)) {
            auto pl = playlists.get_playlist_by_id(id);
            res.set_content(json(*pl).dump(), "application/json");
        } else {
            res.status = 404;
            res.set_content(json{{"error", "Failed to remove track"}}.dump(), "application/json");
        }
    });

    svr.Post(R"(/api/playlists/([a-zA-Z0-9_\-]+)/reorder)", [&](const httplib::Request& req, httplib::Response& res) {
        std::string id = req.matches[1];
        try {
            auto body = json::parse(req.body);
            size_t from_idx = body.value("from_index", 0);
            size_t to_idx = body.value("to_index", 0);
            if (playlists.reorder_playlist_tracks(id, from_idx, to_idx)) {
                auto pl = playlists.get_playlist_by_id(id);
                res.set_content(json(*pl).dump(), "application/json");
            } else {
                res.status = 400;
                res.set_content(json{{"error", "Failed to reorder playlist tracks"}}.dump(), "application/json");
            }
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    // --- Queue ---
    svr.Get("/api/queue", [&](const httplib::Request&, httplib::Response& res) {
        json body = {
            {"tracks", queue.get_queued_tracks()},
            {"count", queue.size()}
        };
        res.set_content(body.dump(), "application/json");
    });

    svr.Post("/api/queue", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string track_id = body.value("track_id", "");
            if (track_id.empty() || !library.get_track_by_id(track_id)) {
                res.status = 400;
                res.set_content(json{{"error", "Invalid track ID"}}.dump(), "application/json");
                return;
            }
            queue.enqueue(track_id);
            json resp = {
                {"success", true},
                {"queue", queue.get_queued_tracks()}
            };
            res.set_content(resp.dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Delete(R"(/api/queue/([a-zA-Z0-9_\-]+))", [&](const httplib::Request& req, httplib::Response& res) {
        std::string track_id = req.matches[1];
        bool removed = queue.remove(track_id);
        json resp = {
            {"success", removed},
            {"queue", queue.get_queued_tracks()}
        };
        res.set_content(resp.dump(), "application/json");
    });

    svr.Post("/api/queue/next", [&](const httplib::Request&, httplib::Response& res) {
        std::string next_id = queue.dequeue();
        if (!next_id.empty()) {
            player.play(next_id);
        }
        json resp = {
            {"current_track_id", next_id},
            {"player", player.get_state()},
            {"queue", queue.get_queued_tracks()}
        };
        res.set_content(resp.dump(), "application/json");
    });

    svr.Post("/api/queue/clear", [&](const httplib::Request&, httplib::Response& res) {
        queue.clear();
        res.set_content(json{{"success", true}}.dump(), "application/json");
    });

    // --- Player ---
    svr.Get("/api/player", [&](const httplib::Request&, httplib::Response& res) {
        json body = player.get_state();
        res.set_content(body.dump(), "application/json");
    });

    svr.Post("/api/player/play", [&](const httplib::Request& req, httplib::Response& res) {
        std::string track_id = "";
        if (!req.body.empty()) {
            try {
                auto body = json::parse(req.body);
                track_id = body.value("track_id", "");
            } catch (...) {}
        }
        player.play(track_id);
        res.set_content(json(player.get_state()).dump(), "application/json");
    });

    svr.Post("/api/player/pause", [&](const httplib::Request&, httplib::Response& res) {
        player.pause();
        res.set_content(json(player.get_state()).dump(), "application/json");
    });

    svr.Post("/api/player/next", [&](const httplib::Request&, httplib::Response& res) {
        player.next();
        res.set_content(json(player.get_state()).dump(), "application/json");
    });

    svr.Post("/api/player/previous", [&](const httplib::Request&, httplib::Response& res) {
        player.previous();
        res.set_content(json(player.get_state()).dump(), "application/json");
    });

    svr.Post("/api/player/seek", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            int pos = body.value("position_sec", 0);
            player.seek(pos);
            res.set_content(json(player.get_state()).dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Post("/api/player/volume", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            int vol = body.value("volume", 80);
            player.set_volume(vol);
            res.set_content(json(player.get_state()).dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Post("/api/player/repeat", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            auto body = json::parse(req.body);
            std::string mode = body.value("mode", "off");
            player.set_repeat_mode(mode);
            res.set_content(json(player.get_state()).dump(), "application/json");
        } catch (...) {
            res.status = 400;
            res.set_content(json{{"error", "Invalid JSON"}}.dump(), "application/json");
        }
    });

    svr.Post("/api/player/shuffle", [&](const httplib::Request&, httplib::Response& res) {
        player.toggle_shuffle();
        res.set_content(json(player.get_state()).dump(), "application/json");
    });

    // --- History & Liked ---
    svr.Get("/api/history", [&](const httplib::Request&, httplib::Response& res) {
        json body = player.get_history();
        res.set_content(body.dump(), "application/json");
    });

    svr.Get("/api/liked", [&](const httplib::Request&, httplib::Response& res) {
        json body = library.get_liked_tracks();
        res.set_content(body.dump(), "application/json");
    });

    // --- Reset ---
    svr.Post("/api/reset", [&](const httplib::Request&, httplib::Response& res) {
        library.reset();
        playlists.reset();
        queue.reset();
        player.reset();
        res.set_content(json{{"status", "reset_successful"}}.dump(), "application/json");
    });

    // Determine listening port
    int port = 8080;
    const char* env_port = std::getenv("PORT");
    if (env_port) {
        try {
            port = std::stoi(env_port);
        } catch (...) {}
    }

    std::cout << "[TuneBox] Starting server on 0.0.0.0:" << port << std::endl;
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "[TuneBox] Failed to listen on port " << port << std::endl;
        return 1;
    }

    return 0;
}
