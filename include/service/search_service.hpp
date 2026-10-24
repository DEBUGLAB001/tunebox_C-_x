#pragma once

#include <vector>
#include <string>
#include "model/music_models.hpp"
#include "service/library_service.hpp"

namespace tunebox {

class SearchService {
public:
    explicit SearchService(const LibraryService& library);

    // Search tracks matching query across title, artist, album, and genre
    std::vector<Track> search(const std::string& query, const std::string& field_filter = "all") const;

    // Helper string utilities
    static std::string trim(const std::string& str);
    static std::string to_lower(const std::string& str);

private:
    const LibraryService& library_;
};

} // namespace tunebox
