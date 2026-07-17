#include "login.h"

#include "auth_store.h"
#include "entities/auth.h"
#include "entities/provider.h"
#include "providers/auth_provider.h"
#include "providers/provider_factory.h"
#include "queries/auth_queries.h"

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

bool attempt_to_refresh_session(AuthProvider* auth_provider, const entities::AuthInfo& auth_info,
                                std::string_view provider_id_str)
{
    std::cout << "Attempting to refresh session...\n";

    std::promise<AuthConnectResult> completion_promise;
    auto completion_future = completion_promise.get_future();
    std::stop_source cancellation;

    auth_provider->refresh(auth_info, cancellation.get_token(), [&](const AuthConnectResult& result)
                           { completion_promise.set_value(result); });

    const auto result = completion_future.get();

    if (result.status == AuthConnectStatus::Ok)
    {
        std::cout << "Session refresh completed, no login was necessary. You can continue using "
                  << provider_id_str << " as a provider.\n";
        return true;
    }

    std::cerr << "Something went wrong while refreshing token, most likely the session is no "
                 "longer valid. Doing a full login...\n";

    return false;
}

int login(AuthProvider* auth_provider, std::string_view provider_id_str)
{

    std::promise<AuthConnectResult> completion_promise;
    auto completion_future = completion_promise.get_future();
    std::stop_source cancellation;

    const auto start_url =
        auth_provider->connect(cancellation.get_token(), [&](const AuthConnectResult& result)
                               { completion_promise.set_value(result); });

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
        std::cerr << ": " << *result.error_msg << '\n';
    }

    return 1;
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
    if (queries::auth::is_auth_info_valid(existing_auth_info))
    {
        std::cout << "You're already logged into " << provider_id_str << '\n';
        return 0;
    }

    auto auth_provider =
        providers::create_auth_provider(provider_id.value(), providers::GlobalDependencies{
                                                                 .auth_store = &store,
                                                             });

    // If the auth info is not valid but still present we can try to refresh the token.
    if (existing_auth_info)
    {
        bool refreshed = attempt_to_refresh_session(auth_provider.get(), existing_auth_info.value(),
                                                    provider_id_str);
        if (refreshed)
        {
            return 0;
        }
    }

    return login(auth_provider.get(), provider_id_str);
}
