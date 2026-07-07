#pragma once

#include <string>

namespace entities
{
struct ArtistId
{
    std::string value;
};

struct Artist
{
    ArtistId id;
    std::string name;
};
} // namespace entities
