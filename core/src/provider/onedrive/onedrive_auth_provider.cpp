#include "auth_store.h"
#include "base64.hpp"
#include "crypto_hash.h"
#include "entities/auth.h"
#include "httplib.hpp"
#include "onedrive_json.h"
#include "onedrive_provider.h"
#include "providers/auth_provider.h"
#include "random_utils.h"
#include "utils.h"
#include <atomic>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stop_token>
#include <thread>

std::string create_redirect_uri()
{
    std::stringstream ss;
    ss << "http://localhost:" << server_listen_port << "/";
    return ss.str();
}

PkcePairs generate_pkce_pairs()
{
    auto code_verifier = random_utils::generate_random_string(64);
    auto sha_code = crypto::sha256(code_verifier);
    auto code_challenge =
        utils::base64_url_encode(base64::to_base64(utils::vector_to_str(sha_code)));

    return PkcePairs{.code_verifier = code_verifier, .code_challenge = code_challenge};
}

OneDriveAuthProvider::OneDriveAuthProvider(const OneDriveConfig& config,
                                           const AuthStore* auth_store)
    : config{config}, auth_store{auth_store}, client{onedrive_auth_host, onedrive_port},
      request_headers{{"Accept", "application/json"}},
      request_base_params{
          {"client_id", config.client_id},
          {"scope", onedrive_auth_scopes},
          {"redirect_uri", create_redirect_uri()},
          {"grant_type", "authorization_code"},
      },
      pkce_pairs(generate_pkce_pairs())
{
}

StartUrl OneDriveAuthProvider::connect(std::stop_token cancellation_token,
                                       std::function<void(AuthConnectResult)> on_complete)
{
    auto completed = std::make_shared<std::atomic<bool>>(false);

    auto _on_complete = [this, completed, on_complete](AuthConnectResult value) mutable
    {
        if (completed->exchange(true))
        {
            return;
        }

        if (value.result.has_value())
        {
            this->auth_store->save(value.result.value());
        }
        on_complete(std::move(value));
        server.stop();
    };

    setup_incoming_server(_on_complete);

    // Create a worker thread that listens to cancellation and holds the server. `_on_complete`
    // is captured by value everywhere: it (and the state it closes over) must outlive `connect()`,
    // which returns as soon as the worker thread is spawned.
    worker = std::jthread(
        [this, cancellation_token, _on_complete]() mutable
        {
            std::thread monitor(
                [cancellation_token, _on_complete]() mutable
                {
                    while (!cancellation_token.stop_requested())
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }

                    _on_complete(AuthConnectResult{.status = AuthConnectStatus::Cancelled});
                });

            server.listen("localhost", server_listen_port);
            monitor.join();
        });

    return create_start_url();
}

void OneDriveAuthProvider::setup_incoming_server(std::function<void(AuthConnectResult)> on_value)
{
    server.Get(
        "/",
        [this, on_value](const httplib::Request& req, httplib::Response& resp) mutable
        {
            if (req.has_param("error"))
            {
                auto error = req.get_param_value("error");
                resp.set_content("Authentication failed, you can close the window.",
                                 utils::content_type_text);
                on_value(AuthConnectResult{.status = AuthConnectStatus::Error, .error_msg = error});
                return;
            }

            if (!req.has_param("code"))
            {
                resp.status = 400;
                resp.set_content("Missing authorization code.", utils::content_type_text);
                return;
            }

            auto code = req.get_param_value("code");
            auto maybe_auth_info = retrieve_auth_info(code);
            if (!maybe_auth_info)
            {
                const auto* error_msg = "Unable to retrieve tokens.";
                on_value(
                    AuthConnectResult{.status = AuthConnectStatus::Error, .error_msg = error_msg});
                resp.set_content(error_msg, utils::content_type_text);
                return;
            }

            resp.set_content("Authentication finished, you can now close the window.",
                             utils::content_type_text);
            on_value(AuthConnectResult{
                .status = AuthConnectStatus::Ok,
                .result = maybe_auth_info,
            });
        });
}

std::optional<entities::AuthInfo> OneDriveAuthProvider::retrieve_auth_info(std::string code)
{
    auto params = request_base_params;
    params.emplace("code", code.data());
    params.emplace("code_verifier", pkce_pairs.code_verifier);

    auto res = client.Post(onedrive_token_req_endpoint, request_headers, params);
    if (!res)
    {
        return std::nullopt;
    }

    if (res->status < 200 || res->status >= 300)
    {
        return std::nullopt;
    }

    return onedrive::parse_auth_info(res->body);
}

std::string OneDriveAuthProvider::create_start_url() const
{
    auto redirect_uri = create_redirect_uri();

    std::ostringstream ss;
    ss << onedrive_auth_start_base_url;
    ss << onedrive_start_endpoint;
    ss << "?client_id=" << utils::url_encode(config.client_id);
    ss << "&response_type=code";
    ss << "&redirect_uri=" << utils::url_encode(redirect_uri);
    ss << "&scope=" << utils::url_encode(onedrive_auth_scopes);
    ss << "&code_challenge=" << pkce_pairs.code_challenge;
    ss << "&code_challenge_method=S256";
    return ss.str();
}
