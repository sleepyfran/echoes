#include "../../utils.h"
#include "base64.hpp"
#include "crypto_provider.h"
#include "entities/auth.h"
#include "httplib.hpp"
#include "onedrive_json.h"
#include "providers/auth_provider.h"
#include "random_utils.h"
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stop_token>
#include <thread>

struct OneDriveConfig
{
    std::string base_url;
    std::string token_req_endpoint;
    std::string start_endpoint;
    uint16_t port;
    uint16_t listening_server_port;
    std::string client_id;
};

class OneDriveAuthProvider : AuthProvider
{
    OneDriveConfig config;
    std::unique_ptr<crypto::CryptoProvider> crypto_provider;
    httplib::Server server;
    httplib::SSLClient client;
    httplib::Headers request_headers;
    httplib::Params request_base_params;
    std::jthread worker;

  public:
    OneDriveAuthProvider(OneDriveConfig& config,
                         std::unique_ptr<crypto::CryptoProvider> crypto_provider)
        : config{config}, crypto_provider{std::move(crypto_provider)},
          client{config.base_url, config.port}, request_headers{{"Accept", "application/json"}},
          request_base_params{
              {"client_id", config.client_id},
              {"scope", "offline_access Files.Read Files.Read.All User.Read"},
              {"redirect_uri", create_redirect_uri()},
              {"grant_type", "authorization_code"},
          }
    {
    }

    StartUrl connect(std::stop_token cancellation_token,
                     std::function<void(AuthConnectResult)> on_complete) override
    {
        std::stop_source worker_stop;

        auto _on_complete = [&worker_stop, &on_complete](AuthConnectResult value)
        {
            worker_stop.request_stop();
            on_complete(std::move(value));
        };

        setup_incoming_server(_on_complete);

        // Create a worker thread that listens to cancellation and holds the server.
        worker = std::jthread(
            [this, cancellation_token, &_on_complete]()
            {
                std::thread monitor(
                    [this, cancellation_token, &_on_complete]
                    {
                        while (!cancellation_token.stop_requested())
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }

                        _on_complete(AuthConnectResult{.status = AuthConnectStatus::Cancelled});
                    });

                server.listen("localhost", this->config.listening_server_port);
                monitor.join();
            });

        return create_start_url();
    }

  private:
    void setup_incoming_server(std::function<void(AuthConnectResult)> on_value)
    {
        server.Get("/",
                   [&](const httplib::Request& req, httplib::Response& resp)
                   {
                       if (req.has_param("error"))
                       {
                           auto error = req.get_param_value("error");
                           resp.set_content("Authentication failed, you can close the window.",
                                            utils::content_type_text);
                           on_value(AuthConnectResult{.status = AuthConnectStatus::Error,
                                                      .error_msg = error});
                           return;
                       }

                       if (!req.has_param("code"))
                       {
                           resp.status = 400;
                           resp.set_content("Missing authorization code.",
                                            utils::content_type_text);
                           return;
                       }

                       auto code = req.get_param_value("code");
                       auto maybe_auth_info = retrieve_auth_info(code);
                       if (!maybe_auth_info)
                       {
                           const auto* error_msg = "Unable to retrieve tokens.";
                           on_value(AuthConnectResult{.status = AuthConnectStatus::Error,
                                                      .error_msg = error_msg});
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

    std::optional<entities::AuthInfo> retrieve_auth_info(std::string code)
    {
        auto params = this->request_base_params;
        params.emplace("code", code.data());

        auto res =
            this->client.Post(this->config.token_req_endpoint, this->request_headers, params);
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

    std::string create_start_url() const
    {
        auto redirect_uri = create_redirect_uri();
        auto code_challenge = this->generate_code_challenge();

        std::ostringstream ss;
        ss << this->config.base_url;
        ss << this->config.start_endpoint;
        ss << "?client_id=" << utils::url_encode(this->config.client_id);
        ss << "&response_type=code";
        ss << "&redirect_uri=" << utils::url_encode(redirect_uri);
        ss << "&scope="
           << utils::url_encode(
                  "offline_access Files.Read Files.Read.All User.Read openid profile email");
        ss << "&code_challenge=" << code_challenge;
        ss << "&code_challenge_method=S256";
        return ss.str();
    }

    std::string generate_code_challenge() const
    {
        auto random_code = random_utils::generate_random_string(64);
        auto sha_code = this->crypto_provider->sha256(random_code);
        return base64::to_base64(utils::vector_to_str(sha_code));
    }

    std::string create_redirect_uri() const
    {
        std::stringstream ss;
        ss << "http://localhost:" << this->config.listening_server_port << "/";
        return ss.str();
    }
};
