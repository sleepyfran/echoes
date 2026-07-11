#pragma once

#include "entities/auth.h"
#include "entities/provider.h"
#include <optional>

/**
 * Abstract class that should be implemented with the native keychain provider of each operating
 * system, which will most likely be KDE Wallet for Linux and Apple's Keychain in macOS.
 */
class AuthStore
{
  public:
    virtual ~AuthStore() = default;

    /**
     * Saves the specified authentication info in the native keychain provider.
     */
    virtual bool save(const entities::AuthInfo& info) const = 0;

    /**
     * Retrieves the associated, previously saved, authentication info, if any, for the given
     * provider ID.
     */
    [[nodiscard]] virtual std::optional<entities::AuthInfo>
    retrieve(entities::ProviderId provider_id) const = 0;
};
