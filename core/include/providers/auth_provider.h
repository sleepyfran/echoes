#pragma once

#include "entities/auth.h"
#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>

enum class AuthConnectStatus : uint8_t
{
    Ok,
    Error,
    Cancelled,
};

/**
 * Result of a connect operation, which can be successful (status OK and result with a value),
 * error (status error and error_msg populated) or cancelled (status cancelled).
 */
struct AuthConnectResult
{
    AuthConnectStatus status;
    std::optional<entities::AuthInfo> result;
    std::optional<std::string> error_msg;
};

using StartUrl = std::string;

constexpr const uint16_t server_listen_port = 8809;

class AuthProvider
{
  public:
    AuthProvider() = default;
    virtual ~AuthProvider() = default;

    /**
     * Performs a connection with the auth provider, returning a future that will contain either the
     * authentication info gathered from the process, an error or a cancelled status if the provided
     * cancellation token is used.
     */
    virtual StartUrl connect(std::stop_token cancellation_token,
                             std::function<void(AuthConnectResult)> on_complete) = 0;
};
