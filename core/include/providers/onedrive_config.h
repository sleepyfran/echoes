#pragma once

#include <cstdint>
#include <string>

constexpr const char* onedrive_host = "login.microsoftonline.com";
constexpr const char* onedrive_base_url = "https://login.microsoftonline.com";
constexpr const char* onedrive_token_req_endpoint = "/common/oauth2/v2.0/token";
constexpr const char* onedrive_start_endpoint = "/common/oauth2/v2.0/authorize";
constexpr const char* onedrive_auth_scopes =
    "offline_access Files.Read Files.Read.All User.Read openid profile email";
constexpr const uint16_t onedrive_port = 443;

struct OneDriveConfig
{
    std::string client_id;
};
