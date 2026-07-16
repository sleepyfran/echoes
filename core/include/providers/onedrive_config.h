#pragma once

#include "entities/file_system.h"
#include <cstdint>
#include <format>
#include <string>

constexpr const char* onedrive_auth_host = "login.microsoftonline.com";
constexpr const char* onedrive_auth_start_base_url = "https://login.microsoftonline.com";
constexpr const char* onedrive_token_req_endpoint = "/common/oauth2/v2.0/token";
constexpr const char* onedrive_start_endpoint = "/common/oauth2/v2.0/authorize";
constexpr const char* onedrive_auth_scopes =
    "offline_access Files.Read Files.Read.All User.Read openid profile email";
constexpr const uint16_t onedrive_port = 443;

constexpr const char* onedrive_graph_host = "graph.microsoft.com";
constexpr const char* onedrive_list_root_endpoint = "/v1.0/me/drive/root/children";
constexpr const char* onedrive_items_endpoint = "/v1.0/me/drive/items";

/**
 * Creates the endpoint to retrieve the contents of a specific folder.
 */
inline std::string onedrive_list_folder_endpoint(entities::ItemId item_id)
{
    return std::format("/me/drive/items/${}/children", item_id.value);
}

struct OneDriveConfig
{
    std::string client_id;
};
