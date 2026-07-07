#pragma once

#include <cstdint>
#include <string>

namespace entities
{
/**
 * Unique ID of a folder or file.
 */
struct ItemId
{
    std::string value;
};

/**
 * Common metadata of a folder or file.
 */
struct ItemMetadata
{
    ItemId id;
    std::string name;
};

/**
 * Metadata specific to a folder.
 */
struct FolderMetadata : ItemMetadata
{
};

/**
 * Metadata specific to a file.
 */
struct FileMetadata : ItemMetadata
{
    uint64_t byte_size;
    std::string mime_type;
    std::string download_url;
};
} // namespace entities
