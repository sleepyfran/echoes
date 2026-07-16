#include "login.h"

#include "auth_store.h"
#include "entities/provider.h"
#include "providers/provider_factory.h"

#include <ctime>
#include <future>
#include <iostream>
#include <optional>
#include <stop_token>

namespace
{

void print_usage()
{
    std::cerr << "Usage: echoes-cli login onedrive\n";
}

} // namespace

int handle_login_command(AuthStore& store, const Args& args)
{
    if (!args.flags.empty() || args.positional.size() != 2)
    {
        print_usage();
        return 1;
    }

    auto provider_id_str = args.positional[1];
    auto provider_id = entities::provider_from_string(provider_id_str);
    if (provider_id == std::nullopt)
    {
        std::cerr << "Unsupported provider: " << provider_id_str << '\n';
        return 1;
    }

    auto existing_auth_info = store.retrieve(provider_id.value());
    if (existing_auth_info.has_value() &&
        existing_auth_info.value().expires_on > std::time(nullptr))
    {
        std::cout << "You're already logged into " << provider_id_str << '\n';
        return 0;
    }

    auto auth_provider =
        providers::create_auth_provider(provider_id.value(), providers::GlobalDependencies{
                                                                 .auth_store = &store,
                                                             });

    std::promise<AuthConnectResult> completion_promise;
    auto completion_future = completion_promise.get_future();
    std::stop_source cancellation;

    const auto start_url = auth_provider->connect(cancellation.get_token(),
                                                  [&](const AuthConnectResult& result)
                                                  {
                                                      completion_promise.set_value(result);
                                                      cancellation.request_stop();
                                                  });

    std::cout << "Open this URL in your browser to sign in:\n" << start_url << '\n';

    const auto result = completion_future.get();

    if (result.status == AuthConnectStatus::Ok)
    {
        std::cout << "Login completed. You can now use " << provider_id_str << " as a provider.\n";
        return 0;
    }

    if (result.status == AuthConnectStatus::Cancelled)
    {
        std::cerr << "Login cancelled.\n";
        return 1;
    }

    std::cerr << "Login failed";
    if (result.error_msg)
    {
        std::cerr << ": " << *result.error_msg;
    }
    std::cerr << '\n';
    return 1;
}
