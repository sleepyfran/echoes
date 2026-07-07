#pragma once

#include "entities/artist.h"
#include <cstdint>
#include <string>
#include <vector>

namespace entities
{
struct TrackId
{
    std::string value;
};

struct Track
{
    TrackId id;
    Artist main_artist;
    std::vector<Artist> secondary_artists;
    std::string title;
    std::uint8_t track_number;
    int duration_in_seconds;
};
} // namespace entities
