#pragma once

#include <array>
#include <cstdint>
#include <ctime>
#include <optional>
#include <string_view>
#include <variant>

namespace entities
{

/**
 * Supported providers that can be currently used in the app.
 */
enum class ProviderId : std::int8_t
{
    OneDrive,
    Spotify,
};

/**
 * Returns the string representation of the given provider ID. This representation is not meant to
 * be displayed directly to the user, but rather just for parsing purposes and other internal usage.
 */
inline std::optional<entities::ProviderId> provider_from_string(std::string_view input)
{
    if (input == "onedrive")
    {
        return entities::ProviderId::OneDrive;
    }

    if (input == "spotify")
    {
        return entities::ProviderId::Spotify;
    }

    return std::nullopt;
}

/**
 * Gathers all available providers in the app.
 */
constexpr std::array<ProviderId, 2> AllProviderIds = {ProviderId::OneDrive, ProviderId::Spotify};

/**
 * Whether the means of interaction with a provider is via a file system (OneDrive, Google Drive,
 * etc.) or via an API (streaming services like Spotify).
 */
enum ProviderType : std::int8_t
{
    FileBased,
    ApiBased
};

struct ProviderMetadata
{
    ProviderId id;
    ProviderType type;
};

/**
 * Status sentinel for when a provider has not been started yet.
 */
struct ProviderStatusNotStarted
{
};

/**
 * Status sentinel for when a provider is in the middle of a sync.
 */
struct ProviderStatusSyncing
{
};

/**
 * Status for when a provider has finished syncing, with information about how the sync went.
 */
struct ProviderStatusSynced
{
    std::time_t last_sync;
    int synced_tracks;
    int errored_tracks;
};

/**
 * Status for when a provider has skipped syncing due to the last sync being recent. Attaches a last
 * synced time.
 */
struct ProviderStatusSyncSkipped
{
    std::time_t last_synced_at;
};

enum ProviderError : std::int8_t
{
    TokenExpired,
    ApiGatewayError,
};

/**
 * Status for when a provider has errored during the sync with information about it.
 */
struct ProviderStatusErrored
{
    ProviderError error;
};

/**
 * Sentinel status for when a provider has been stopped by the user.
 */
struct ProviderStatusStopped
{
};

/**
 * Holds all the possible current statuses of a provider.
 */
using ProviderStatus =
    std::variant<ProviderStatusNotStarted, ProviderStatusSyncing, ProviderStatusSynced,
                 ProviderStatusSyncSkipped, ProviderStatusErrored, ProviderStatusStopped>;
} // namespace entities
