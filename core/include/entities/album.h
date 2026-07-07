#pragma once

#include "entities/artist.h"
#include "entities/track.h"
#include <chrono>
#include <string>

namespace entities
{
struct AlbumId
{
    std::string value;
};

struct Album
{
    AlbumId id;
    Artist artist;
    std::string title;
    std::chrono::year release_year;
    std::vector<std::string> genres;
    std::vector<Track> tracks;
};
} // namespace entities
