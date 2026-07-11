#pragma once

#include "auth_store.h"
#include "crypto_provider.h"
#include "entities/provider.h"
#include "providers/auth_provider.h"
#include <memory>

namespace providers
{
/**
 * Platform-supplied dependencies that are required to construct an auth provider but that core
 * itself can't implement (e.g. crypto primitives backed by OS-specific APIs).
 */
struct GlobalDependencies
{
    crypto::CryptoProvider* crypto_provider;
    AuthStore* auth_store;
};

/**
 * Creates the associated auth provider for the given provider ID.
 */
std::unique_ptr<AuthProvider> create_auth_provider(entities::ProviderId id,
                                                   GlobalDependencies deps);
} // namespace providers
