#pragma once

#include "entities/file_system.h"
#include "entities/provider.h"
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

class MediaProvider
{
  public:
    entities::ProviderId id;
    entities::ProviderStatus status;

    MediaProvider(entities::ProviderId id) : id{id} {};
    virtual ~MediaProvider() = default;
};

/**
 * Virtual class that should be implemented by providers that use file-based operations like
 * OneDrive.
 */
class FileBasedProvider : public MediaProvider
{
  public:
    FileBasedProvider(entities::ProviderId id) : MediaProvider(id) {};
    ~FileBasedProvider() override = default;

    /**
     * Lists the root folder of the provider.
     */
    [[nodiscard]] virtual FolderContentResult list_root() = 0;

    /**
     * Lists the contents of a specific folder.
     */
    [[nodiscard]] virtual FolderContentResult
    list_folder(const entities::FolderMetadata& folder) = 0;

    /**
     * Returns the URL that points to a specific item by its ID.
     */
    [[nodiscard]] virtual UrlResult file_url_by_id(const entities::ItemId& id) = 0;
};
} // namespace media_provider
