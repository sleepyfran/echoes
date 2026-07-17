#pragma once

#include "entities/file_system.h"
#include <cstdint>
#include <vector>

namespace media_provider
{
/**
 * Defines the statuses that can be returned from any of the
 */
enum class MediaProviderResultStatus : uint8_t
{
    Ok = 0,
    MissingAuth,
    ApiError,
};

enum class MediaProviderError : uint16_t
{
    Unknown = 0,
    Unauthorized = 401
};

template <typename T> struct MediaProviderResult
{
    MediaProviderResultStatus status;
    T result;
    MediaProviderError error;
};

using FolderContent = std::vector<entities::ItemMetadata>;
using FolderContentResult = MediaProviderResult<FolderContent>;
using UrlResult = MediaProviderResult<std::string>;

/**
 * Virtual class that should be implemented by providers that use file-based operations like
 * OneDrive.
 */
class FileBasedProvider
{
  public:
    FileBasedProvider() = default;
    virtual ~FileBasedProvider() = default;

    /**
     * Lists the root folder of the provider.
     */
    virtual FolderContentResult list_root() = 0;

    /**
     * Lists the contents of a specific folder.
     */
    virtual FolderContentResult list_folder(entities::FolderMetadata& folder) = 0;

    /**
     * Returns the URL that points to a specific item by its ID.
     */
    virtual UrlResult file_url_by_id(entities::ItemId& id) = 0;
};
} // namespace media_provider
