#pragma once

#include <cstdint>
#include <string>

/**
 * Configuration needed to talk to the OneDrive/MSAL endpoints. Public (rather than living
 * alongside `OneDriveAuthProvider`) so that platform-side callers can construct it when
 * requesting an auth provider from `providers::create_auth_provider`.
 */
struct OneDriveConfig
{
    std::string base_url;
    std::string token_req_endpoint;
    std::string start_endpoint;
    uint16_t port;
    uint16_t listening_server_port;
    std::string client_id;
};
