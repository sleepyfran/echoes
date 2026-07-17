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
 * Message sent to a previously started provider to stop its operation. No-op if the provider is not
 * started or in a non-syncing state.
 */
struct SyncWorkerStopMessage
{
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
using SyncWorkerMessage =
    std::variant<SyncWorkerStartMessage, SyncWorkerStopMessage, SyncWorkerForceSyncMessage>;

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
 * Defines an event that can be raised by a sync worker to notify the main thread of something that
 * happened during its lifetime.
 */
using SyncWorkerEvent = std::variant<SyncWorkerEventStatusChanged>;
} // namespace entities
