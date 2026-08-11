#pragma once

#include "entities/file_system.h"
#include "entities/provider.h"
#include <cstdint>
#include <variant>

namespace entities
{
struct FileBasedProviderStartArgs
{
    FolderMetadata selected_folder;
};

struct ApiBasedProviderStartArgs
{
    // Placeholder, we might need stuff later on.
};

struct StartArgs
{
    entities::ProviderId provider_id;
    std::variant<FileBasedProviderStartArgs, ApiBasedProviderStartArgs> args;
};

/**
 * Message sent when a provider should wake up with the following start args to perform a sync.
 * No-op if the provider is already started.
 */
struct SyncWorkerStartMessage
{
    StartArgs args;
};

/**
 * Message sent when a provider should perform a sync regardless of when the last sync was.
 */
struct SyncWorkerForceSyncMessage
{
};

/**
 * Defines a message that can be passed onto a sync worker to modify its status.
 */
using SyncWorkerMessage = std::variant<SyncWorkerStartMessage, SyncWorkerForceSyncMessage>;

/**
 * Defines the types of events that a sync worker can raise during its lifetime.
 */
enum class SyncWorkerEventType : uint8_t
{
    ProviderStatusChanged = 0,
    ProviderAuthChanged,
};

/**
 * Sync worker event raised when the status of the underlying provider changed.
 */
struct SyncWorkerEventStatusChanged
{
    entities::ProviderStatus previous;
    entities::ProviderStatus current;
};

/**
 * Sync worker event raised when a folder could not be enumerated during a sync, along with the
 * reason why.
 */
struct SyncWorkerFolderSkippedDuringSync
{
    entities::FolderMetadata folder;
    entities::ProviderError error;
};

/**
 * Sync worker event raised when a file has been processed by the sync process.
 */
struct SyncWorkerFileProcessedDuringSync
{
    entities::FileMetadata file;
};

/**
 * Defines an event that can be raised by a sync worker to notify the main thread of something that
 * happened during its lifetime.
 */
using SyncWorkerEvent =
    std::variant<SyncWorkerEventStatusChanged, SyncWorkerFolderSkippedDuringSync,
                 SyncWorkerFileProcessedDuringSync>;
} // namespace entities
