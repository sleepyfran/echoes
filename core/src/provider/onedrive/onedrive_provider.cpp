#include "../../utils.h"
#include "entities/auth.h"
#include "httplib.hpp"
#include "onedrive_json.h"
#include "providers/auth_provider.h"
#include <cstdint>
#include <future>
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
    httplib::Server server;
    httplib::SSLClient client;
    httplib::Headers request_headers;
    httplib::Params request_base_params;
    std::jthread worker;

  public:
    OneDriveAuthProvider(OneDriveConfig& config)
        : config{config}, client{config.base_url, config.port},
          request_headers{{"Accept", "application/json"}},
          request_base_params{
              {"client_id", config.client_id},
              {"scope", "offline_access Files.Read Files.Read.All User.Read"},
              {"redirect_uri", create_redirect_uri()},
              {"grant_type", "authorization_code"},
          }
    {
    }

    ConnectResult connect(std::stop_token cancellation_token) override
    {
        auto promise = std::make_shared<std::promise<AuthConnectResult>>();
        auto future = promise->get_future();
        std::stop_source worker_stop;

        setup_incoming_server(promise, worker_stop);

        // Create a worker thread that listens to cancellation and holds the server.
        worker = std::jthread(
            [this, promise, cancellation_token]()
            {
                std::thread monitor(
                    [this, cancellation_token, promise]
                    {
                        while (!cancellation_token.stop_requested())
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(100));
                        }

                        try
                        {
                            promise->set_value(AuthConnectResult{.status = AuthConnectStatus::Ok});
                        }
                        catch (...)
                        {
                            // TODO: Do something?
                        }
                    });

                server.listen("localhost", this->config.listening_server_port);
                monitor.join();
            });

        return {.start_url = create_start_url(), .future = std::move(future)};
    }

  private:
    void setup_incoming_server(std::shared_ptr<std::promise<AuthConnectResult>>& promise,
                               std::stop_source& worker_stop)
    {
        server.Get("/",
                   [&](const httplib::Request& req, httplib::Response& resp)
                   {
                       if (req.has_param("error"))
                       {
                           auto error = req.get_param_value("error");
                           resp.set_content("Authentication failed, you can close the window.",
                                            utils::content_type_text);
                           promise->set_value(AuthConnectResult{.status = AuthConnectStatus::Error,
                                                                .error_msg = error});
                           worker_stop.request_stop();
                           return;
                       }

                       if (!req.has_param("code"))
                       {
                           resp.status = 400;
                           resp.set_content("Missing authorization code.",
                                            utils::content_type_text);
                           worker_stop.request_stop();
                           return;
                       }

                       auto code = req.get_param_value("code");
                       auto maybe_auth_info = retrieve_auth_info(code);
                       if (!maybe_auth_info)
                       {
                           const auto* error_msg = "Unable to retrieve tokens.";
                           promise->set_value(AuthConnectResult{.status = AuthConnectStatus::Error,
                                                                .error_msg = error_msg});
                           resp.set_content(error_msg, utils::content_type_text);
                           return;
                       }

                       resp.set_content("Authentication finished, you can now close the window.",
                                        utils::content_type_text);
                       promise->set_value(AuthConnectResult{
                           .status = AuthConnectStatus::Ok,
                           .result = maybe_auth_info,
                       });
                       worker_stop.request_stop();
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

        std::ostringstream ss;
        ss << this->config.base_url;
        ss << this->config.start_endpoint;
        ss << "?client_id=" << utils::url_encode(this->config.client_id);
        ss << "&response_type=code";
        ss << "&redirect_uri=" << utils::url_encode(redirect_uri);
        ss << "&scope="
           << utils::url_encode(
                  "offline_access Files.Read Files.Read.All User.Read openid profile email");
        return ss.str();
    }

    std::string create_redirect_uri() const
    {
        std::stringstream ss;
        ss << "http://localhost:" << this->config.listening_server_port << "/";
        return ss.str();
    }
};
