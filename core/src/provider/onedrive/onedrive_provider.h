#pragma once

#include "auth_store.h"
#include "crypto_provider.h"
#include "httplib.hpp"
#include "providers/auth_provider.h"
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
    const crypto::CryptoProvider* crypto_provider;
    const AuthStore* auth_store;
    httplib::Server server;
    httplib::SSLClient client;
    httplib::Headers request_headers;
    httplib::Params request_base_params;
    std::jthread worker;
    PkcePairs pkce_pairs;

  public:
    OneDriveAuthProvider(const OneDriveConfig& config,
                         const crypto::CryptoProvider* crypto_provider,
                         const AuthStore* auth_store);

    StartUrl connect(std::stop_token cancellation_token,
                     std::function<void(AuthConnectResult)> on_complete) override;

  private:
    void setup_incoming_server(std::function<void(AuthConnectResult)> on_value);
    std::optional<entities::AuthInfo> retrieve_auth_info(std::string code);
    PkcePairs generate_pkce_pairs() const;
    std::string create_start_url() const;
};
