#pragma once

#include "auth_store.h"
#include "entities/file_system.h"
#include "httplib.hpp"
#include "providers/auth_provider.h"
#include "providers/media_provider.h"
#include "providers/onedrive_config.h"
#include <functional>
#include <stop_token>

struct PkcePairs
{
    std::string code_verifier;
    std::string code_challenge;
};

class OneDriveAuthProvider : public AuthProvider
{
  private:
    OneDriveConfig config;
    const AuthStore* auth_store;
    httplib::Server server;
    httplib::SSLClient client;
    httplib::Headers request_headers;
    httplib::Params request_base_params;
    std::jthread worker;
    PkcePairs pkce_pairs;

  public:
    OneDriveAuthProvider(const OneDriveConfig& config, const AuthStore* auth_store);

    StartUrl connect(std::stop_token cancellation_token,
                     std::function<void(AuthConnectResult)> on_complete) override;
    void refresh(const entities::AuthInfo& original_auth_info, std::stop_token cancellation_token,
                 std::function<void(AuthConnectResult)> on_complete) override;

  private:
    void setup_incoming_server(std::function<void(AuthConnectResult)> on_value);
    std::optional<entities::AuthInfo> retrieve_auth_info(std::string_view code);
    std::optional<entities::AuthInfo> retrieve_refreshed_auth_info(std::string_view refresh_token);
    std::string create_start_url() const;
};

namespace media_provider
{
class OneDriveMediaProvider : public FileBasedProvider
{
  private:
    AuthStore* auth_store;
    httplib::SSLClient client;
    httplib::Headers base_headers;

  public:
    OneDriveMediaProvider(AuthStore* auth_store);
    FolderContentResult list_root() override;
    FolderContentResult list_folder(entities::FolderMetadata& folder) override;
    UrlResult file_url_by_id(entities::ItemId& id) override;
};
} // namespace media_provider
