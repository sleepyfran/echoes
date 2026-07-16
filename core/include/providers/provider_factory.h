#pragma once

#include "auth_store.h"
#include "entities/provider.h"
#include "providers/auth_provider.h"
#include "providers/media_provider.h"
#include <memory>

namespace providers
{
/**
 * Platform-supplied dependencies that are required to construct an auth provider but that core
 * itself can't implement (e.g. crypto primitives backed by OS-specific APIs).
 */
struct GlobalDependencies
{
    AuthStore* auth_store;
};

/**
 * Creates the associated auth provider for the given provider ID.
 */
std::unique_ptr<AuthProvider> create_auth_provider(entities::ProviderId, GlobalDependencies);

/**
 * Creates a file-based provider from the given provider ID. If the provider ID is not associated
 * with a file-based one, it returns null.
 * TODO: Would we need this? Should we hide it behind a shared provider?
 */
std::optional<std::unique_ptr<media_provider::FileBasedProvider>>
    create_file_based_provider(entities::ProviderId, GlobalDependencies);
} // namespace providers
