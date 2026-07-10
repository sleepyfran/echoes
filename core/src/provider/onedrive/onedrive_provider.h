#pragma once

#include "crypto_provider.h"
#include "httplib.hpp"
#include "providers/auth_provider.h"
#include "providers/onedrive_config.h"
#include <functional>
#include <memory>
#include <stop_token>

class OneDriveAuthProvider : public AuthProvider
{
  private:
    OneDriveConfig config;
    std::unique_ptr<crypto::CryptoProvider> crypto_provider;
    httplib::Server server;
    httplib::SSLClient client;
    httplib::Headers request_headers;
    httplib::Params request_base_params;
    std::jthread worker;

  public:
    OneDriveAuthProvider(const OneDriveConfig& config,
                         std::unique_ptr<crypto::CryptoProvider> crypto_provider);

    StartUrl connect(std::stop_token cancellation_token,
                     std::function<void(AuthConnectResult)> on_complete) override;

  private:
    void setup_incoming_server(std::function<void(AuthConnectResult)> on_value);
    std::optional<entities::AuthInfo> retrieve_auth_info(std::string code);
    std::string create_start_url() const;
    std::string generate_code_challenge() const;
};
