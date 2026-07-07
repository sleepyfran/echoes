#pragma once

#include "./provider.h"
#include <string>
#include <variant>

namespace entities
{
/**
 * Authentication info that is needed to authenticate against MSAL, the authentication that OneDrive
 * uses.
 */
struct MsalSpecificAuthInfo
{
    std::string home_account_id;
    std::string environment;
    std::string tenant_id;
    std::string username;
    std::string local_account_id;
};

/**
 * Authentication info that is needed to authenticate against Spotify.
 */
struct SpotifySpecificAuthInfo
{
    std::string refresh_token;
};

/**
 * Authentication info that we store and retrieve in order to authenticate against providers.
 */
struct AuthInfo
{
    ProviderId provider_id;
    std::string access_token;
    std::variant<MsalSpecificAuthInfo, SpotifySpecificAuthInfo> provider_specific_auth_info;
    std::time_t expires_on;
};
} // namespace entities
