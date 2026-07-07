#pragma once

#include "entities/file_system.h"
#include <future>

class FileBasedProvider
{
  public:
    FileBasedProvider() = default;
    virtual ~FileBasedProvider() = default;

    /**
     * Lists the root folder of the provider.
     */
    virtual std::future<std::vector<entities::ItemMetadata>> list_root() = 0;

    /**
     * Lists the contents of a specific folder.
     */
    virtual std::future<std::vector<entities::ItemMetadata>>
    list_folder(entities::FolderMetadata& folder) = 0;

    /**
     * Returns the URL that points to a specific item by its ID.
     */
    virtual std::future<std::string> file_url_by_id(entities::ItemId& id) = 0;
};
