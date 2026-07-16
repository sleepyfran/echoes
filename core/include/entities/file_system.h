#pragma once

#include <cstdint>
#include <string>
#include <variant>

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
struct BaseMetadata
{
    ItemId id;
    std::string name;
    uint64_t byte_size;
};

/**
 * Metadata specific to a folder.
 */
struct FolderMetadata : public BaseMetadata
{
    uint32_t child_count;
};

/**
 * Metadata specific to a file.
 */
struct FileMetadata : public BaseMetadata
{
    std::string mime_type;
    std::string download_url;
};

using ItemMetadata = std::variant<FileMetadata, FolderMetadata>;

/**
 * Returns the common metadata shared by every alternative of ItemMetadata,
 * without requiring callers to visit the variant themselves.
 */
inline const BaseMetadata& base_metadata(const ItemMetadata& item)
{
    return std::visit([](const auto& alternative) -> const BaseMetadata& { return alternative; },
                       item);
}
} // namespace entities
