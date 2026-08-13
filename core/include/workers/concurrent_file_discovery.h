#pragma once

#include "concurrency.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "providers/media_provider.h"
#include <functional>
#include <memory>
#include <optional>
#include <span>
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

using ChunkReadyCallback = std::function<void(std::span<entities::FileMetadata>)>;

struct Subscription
{
    ChunkReadyCallback callback;
    size_t last_notified_chunk_idx;
    size_t max_chunk_size;
};

class ConcurrentFileDiscovery : concurrency::Concurrently<FileDiscoveryJob, std::monostate>
{
  private:
    media_provider::FileBasedProvider& provider;
    std::unique_ptr<std::mutex> file_mutex_;
    std::unique_ptr<std::mutex> subscription_mutex_;
    std::vector<entities::FileMetadata> gathered_files_;
    std::vector<FileDiscoveryError> errors_;
    std::optional<Subscription> subscription_;

    std::monostate run(const FileDiscoveryJob& job) override;
    void on_done() override;

    void maybe_notify_subscription();
    void on_error_received(const entities::FolderMetadata&, const entities::ProviderError&);
    void on_file_received(entities::FileMetadata&);
    void on_files_received(std::vector<entities::FileMetadata>&);

  public:
    ConcurrentFileDiscovery(media_provider::FileBasedProvider& provider,
                            size_t concurrent_requests = 4)
        : concurrency::Concurrently<FileDiscoveryJob, std::monostate>(concurrent_requests),
          subscription_mutex_(std::make_unique<std::mutex>()),
          file_mutex_(std::make_unique<std::mutex>()), provider{provider} {};

    /**
     * Queues the discovery of the given folder. This will start enumerating files and folders
     * contained in it and recursively explore all the subfolders until it gathers every file. After
     * calling this, the caller can be blocked on `wait_for_all_files` to receive a result.
     */
    void queue_discovery(entities::FolderMetadata);

    /**
     * Blocks until all jobs have finished and returns all gathered files and errors during the
     * discovery.
     *
     * In order to receive files as they happen, use `subscribe`.
     */
    FileDiscoveryResult wait_for_all_files();

    /**
     * Subscribes to new files that have been discovered as they happen, with the callback getting
     * called in batches with the contents as they're discovered, chunked by the
     * given N (which defaults to 10).
     *
     * NOTE: Subscriptions are unique, meaning there can only be one subscription for this specific
     * discovery. To stop new events from arriving, call `unsubscribe` or replace the previous
     * callback with a new one using this same method. IMPORTANT: In order to wait for the actual
     * result, meaning blocking until all discovery jobs have been processed, a call to
     * `wait_for_all_files` is still needed.
     *
     * TODO: Should we surface errors through here or leave it on the discovery result?
     */
    void subscribe(ChunkReadyCallback on_chunk_ready, size_t max_chunk_size = 5);

    /**
     * Removes a previous subscription from receiving further updates.
     */
    void unsubscribe();
};
} // namespace file_discovery
