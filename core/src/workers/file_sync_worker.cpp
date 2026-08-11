#include "workers/file_sync_worker.h"
#include "entities/file_system.h"
#include "entities/provider.h"
#include "entities/sync_messages.h"
#include "httplib.hpp"
#include "providers/media_provider.h"
#include "utils.h"
#include <ctime>
#include <optional>
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

entities::ProviderError map_to_provider_error(const media_provider::MediaProviderError& error)
{
    switch (error)
    {
    case media_provider::MediaProviderError::Unauthorized:
        return entities::ProviderError::TokenExpired;
    default:
        return entities::ProviderError::ApiGatewayError;
    }
}

std::optional<std::vector<entities::FileMetadata>> recursively_list_files(
    const Publisher<entities::SyncWorkerEvent>& pubsub, const entities::FolderMetadata& root_folder,
    media_provider::FileBasedProvider& provider, const std::stop_token& cancellation_token)
{
    auto contents = provider.list_folder(root_folder);
    if (contents.status != media_provider::MediaProviderResultStatus::Ok)
    {
        pubsub(entities::SyncWorkerFolderSkippedDuringSync{
            .folder = root_folder, .error = map_to_provider_error(contents.error)});
        return std::nullopt;
    }

    // TODO: Paralellize this.
    std::vector<entities::FileMetadata> gathered_files;
    for (auto item : contents.result)
    {
        if (std::holds_alternative<entities::FileMetadata>(item))
        {
            gathered_files.push_back(std::get<entities::FileMetadata>(item));
        }
        else if (std::holds_alternative<entities::FolderMetadata>(item))
        {
            auto child_files = recursively_list_files(
                pubsub, std::get<entities::FolderMetadata>(item), provider, cancellation_token);
            if (child_files)
            {
                auto files = child_files.value();
                gathered_files.insert(gathered_files.end(), std::begin(files), std::end(files));
            }
        }
    }

    return gathered_files;
}

void sync_file_based_provider(const Publisher<entities::SyncWorkerEvent>& pubsub,
                              downloader::Downloader& downloader,
                              const entities::FolderMetadata& root_folder,
                              media_provider::FileBasedProvider& provider,
                              const std::stop_token& cancellation_token)
{
    if (cancellation_token.stop_requested())
    {
        on_cancelled(pubsub);
        return;
    }

    auto files = recursively_list_files(pubsub, root_folder, provider, cancellation_token);
    if (!files)
    {
        pubsub(
            entities::SyncWorkerEventStatusChanged{.previous = entities::ProviderStatusSyncing(),
                                                   .current = entities::ProviderStatusErrored()});
        return;
    }

    std::atomic<size_t> successful_processed{0};
    std::atomic<size_t> error_processed{0};
    for (auto& file : files.value())
    {
        if (!is_supported_file(file))
        {
            continue;
        }

        downloader.queue(
            file.download_url,
            [&pubsub, &file, &successful_processed, &error_processed](auto result)
            {
                if (std::holds_alternative<std::string_view>(result))
                {
                    successful_processed.fetch_add(1, std::memory_order_relaxed);
                    pubsub(entities::SyncWorkerFileProcessedDuringSync{file});
                }
                else
                {
                    error_processed.fetch_add(1, std::memory_order_relaxed);
                    pubsub(entities::SyncWorkerFileSkippedDuringSync{
                        .file = file, .error = entities::FileProcessingError::DownloadFailed});
                }
            });
    }

    downloader.wait_for_all_queued();

    pubsub(entities::SyncWorkerEventStatusChanged{
        .previous = entities::ProviderStatusSyncing(),
        .current = entities::ProviderStatusSynced{.last_sync = time(nullptr),
                                                  .synced_tracks = successful_processed.load(),
                                                  .errored_tracks = error_processed.load()}});
}
} // namespace file_sync
