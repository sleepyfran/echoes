#pragma once

#include "concurrency.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "providers/media_provider.h"
#include <memory>
#include <variant>

namespace file_discovery
{
struct FileDiscoveryError
{
    entities::FolderMetadata containing_folder;
    entities::ProviderError error;
};

struct FileDiscoveryJob
{
    entities::FolderMetadata folder;
};

struct FileDiscoveryResult
{
    std::vector<entities::FileMetadata> files;
    std::vector<FileDiscoveryError> errors;
};

class ConcurrentFileDiscovery : concurrency::Concurrently<FileDiscoveryJob, std::monostate>
{
  private:
    media_provider::FileBasedProvider& provider;
    std::unique_ptr<std::mutex> file_mutex_;
    std::vector<entities::FileMetadata> gathered_files_;
    std::vector<FileDiscoveryError> errors_;

    std::monostate run(const FileDiscoveryJob& job) override;
    void on_file_received(entities::FileMetadata&);
    void on_files_received(std::vector<entities::FileMetadata>&);

  public:
    ConcurrentFileDiscovery(media_provider::FileBasedProvider& provider,
                            size_t concurrent_requests = 4)
        : concurrency::Concurrently<FileDiscoveryJob, std::monostate>(concurrent_requests),
          file_mutex_(std::make_unique<std::mutex>()),
          provider{provider} {};

    /**
     * Queues the discovery of the given folder. This will start enumerating files and folders
     * contained in it and recursively explore all the subfolders until it gathers every file. After
     * calling this, the caller can be blocked on `wait_for_all_files` to receive a result.
     */
    void queue_discovery(entities::FolderMetadata);

    /**
     * Blocks until all jobs have finished and returns all gathered files and errors during the
     * discovery.
     */
    FileDiscoveryResult wait_for_all_files();
};
} // namespace file_discovery
