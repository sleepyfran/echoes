#pragma once

#include "entities/sync_messages.h"
#include "providers/media_provider.h"
#include "pubsub.h"
#include "workers/downloader.h"
#include <stop_token>
namespace file_sync
{
/**
 * Starts the sync of a file based provider, publishing into the given pub-sub as it advances and
 * attempting to respect the given cancellation token during multiple checkpoints (after each file
 * download, each new file saved, etc.).
 */
void sync_file_based_provider(const Publisher<entities::SyncWorkerEvent>& pubsub,
                              downloader::Downloader& downloader,
                              const entities::FolderMetadata& root_folder,
                              media_provider::FileBasedProvider& provider,
                              const std::stop_token& cancellation_token);
} // namespace file_sync
