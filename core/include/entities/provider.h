#pragma once

#include <array>
#include <cstdint>
#include <ctime>
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
 * The current status that a provider is in.
 */
enum ProviderStatusKind : std::int8_t
{
    /** The provider has yet to be started. */
    NotStarted,
    /** The provider is currently syncing. */
    Syncing,
    /** The provider has finished syncing successfully, although some of the tracks might have
       failed. */
    Synced,
    /** The provider has not synced because it was last synced recently. */
    SyncSkipped,
    /** The provider has errored while syncing and nothing has been retrieved. */
    Errored,
    /** The provider has been stopped. */
    Stopped
};

struct ProviderStatusSyncedData
{
    std::time_t last_sync;
    int synced_tracks;
    int errored_tracks;
};

struct ProviderStatusSyncSkippedData
{
    std::time_t last_synced_at;
};

enum ProviderError : std::int8_t
{
    TokenExpired,
    ApiGatewayError,
};

struct ProviderErroredData
{
    ProviderError error;
};

/**
 * Holds the current status of a provider, which can be of `ProviderStatusKind`, with
 * optional associated data.
 */
struct ProviderStatus
{
    ProviderStatusKind kind;
    std::variant<ProviderStatusSyncedData, ProviderStatusSyncSkippedData, ProviderErroredData> data;
};
} // namespace entities
