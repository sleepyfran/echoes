#include "workers/file_sync_worker.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include <ctime>
#include <regex>
#include <stop_token>
#include <variant>

namespace file_sync
{
void on_cancelled(const Publisher<entities::SyncWorkerEvent>& pubsub)
{
    pubsub(entities::SyncWorkerEventStatusChanged{.previous = entities::ProviderStatusSyncing{},
                                                  .current = entities::ProviderStatusStopped{}});
}

std::regex supported_audio_extensions("\\.(wav|mp3|aac|ogg|flac|m4a|opus)$",
                                      std::regex_constants::icase);
bool is_supported_file(entities::FileMetadata& file)
{
    return std::regex_search(file.name, supported_audio_extensions);
}

void sync_file_based_provider(const Publisher<entities::SyncWorkerEvent>& pubsub,
                              downloader::Downloader& downloader,
                              file_discovery::ConcurrentFileDiscovery& discovery,
                              const entities::FolderMetadata& root_folder,
                              const std::stop_token& cancellation_token)
{
    if (cancellation_token.stop_requested())
    {
        on_cancelled(pubsub);
        return;
    }

    std::atomic<size_t> successful_processed{0};
    std::atomic<size_t> error_processed{0};
    discovery.subscribe(
        [&successful_processed, &error_processed, &downloader,
         &pubsub](std::span<entities::FileMetadata> chunk)
        {
            for (auto& file : chunk)
            {
                if (!is_supported_file(file))
                {
                    continue;
                }

                downloader.queue_download(
                    file.download_url, 50000,
                    [&pubsub, file, &successful_processed, &error_processed](auto& result)
                    {
                        if (std::holds_alternative<std::string>(result))
                        {
                            successful_processed.fetch_add(1, std::memory_order_relaxed);
                            pubsub(entities::SyncWorkerFileProcessedDuringSync{file});
                        }
                        else
                        {
                            error_processed.fetch_add(1, std::memory_order_relaxed);
                            pubsub(entities::SyncWorkerFileSkippedDuringSync{
                                .file = file,
                                .error = entities::FileProcessingError::DownloadFailed});
                        }
                    });
            }
        });

    discovery.queue_discovery(root_folder);
    auto result = discovery.wait_for_all_files();
    if (result.files.size() == 0)
    {
        pubsub(
            entities::SyncWorkerEventStatusChanged{.previous = entities::ProviderStatusSyncing(),
                                                   .current = entities::ProviderStatusErrored()});
        return;
    }

    for (auto& error : result.errors)
    {
        pubsub(entities::SyncWorkerFolderSkippedDuringSync{.folder = error.containing_folder,
                                                           .error = error.error});
    }

    downloader.wait_for_all_downloads();

    pubsub(entities::SyncWorkerEventStatusChanged{
        .previous = entities::ProviderStatusSyncing(),
        .current = entities::ProviderStatusSynced{.last_sync = time(nullptr),
                                                  .synced_tracks = successful_processed.load(),
                                                  .errored_tracks = error_processed.load()}});
}
} // namespace file_sync
