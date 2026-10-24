#include "service/search_service.hpp"
#include <cctype>
#include <algorithm>

namespace tunebox {

SearchService::SearchService(const LibraryService& library)
    : library_(library) {}

std::string SearchService::trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::string SearchService::to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

std::vector<Track> SearchService::search(const std::string& query, const std::string& field_filter) const {
    std::string clean_query = query;
#ifdef SEED_BUGS
    // Bug 2: String Processing defect - fails to normalize whitespace and lowercase query
    // This causes case-insensitive matching and queries with leading/trailing spaces to fail
#else
    clean_query = trim(to_lower(query));
#endif

    if (clean_query.empty()) {
        return {};
    }

    auto all_tracks = library_.get_all_tracks();
    std::vector<Track> matches;

    for (const auto& track : all_tracks) {
        bool matched = false;

#ifdef SEED_BUGS
        auto field_matches = [&clean_query](const std::string& field) {
            return field.find(clean_query) != std::string::npos;
        };
#else
        auto field_matches = [&clean_query](const std::string& field) {
            std::string lower_field = to_lower(field);
            return lower_field.find(clean_query) != std::string::npos;
        };
#endif

        if (field_filter == "title") {
            matched = field_matches(track.title);
        } else if (field_filter == "artist") {
            matched = field_matches(track.artist);
        } else if (field_filter == "album") {
            matched = field_matches(track.album);
        } else if (field_filter == "genre") {
            matched = field_matches(track.genre);
        } else {
            // "all" or unrecognized
            matched = field_matches(track.title) ||
                      field_matches(track.artist) ||
                      field_matches(track.album) ||
                      field_matches(track.genre);
        }

        if (matched) {
            matches.push_back(track);
        }
    }

    return matches;
}

} // namespace tunebox
