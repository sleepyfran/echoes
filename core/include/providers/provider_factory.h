#pragma once

#include "crypto_provider.h"
#include "entities/provider.h"
#include "providers/auth_provider.h"
#include <memory>
#include <variant>

namespace providers
{
/**
 * Platform-supplied dependencies that are required to construct an auth provider but that core
 * itself can't implement (e.g. crypto primitives backed by OS-specific APIs).
 */
struct GlobalDependencies
{
    std::unique_ptr<crypto::CryptoProvider> crypto_provider;
};

/**
 * Creates the associated auth provider for the given provider ID.
 */
std::unique_ptr<AuthProvider> create_auth_provider(entities::ProviderId id, GlobalDependencies deps);
} // namespace providers
