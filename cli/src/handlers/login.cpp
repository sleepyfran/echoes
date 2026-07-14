#include "login.h"

#include "../in_memory_auth_store.h"
#include "../openssl_crypto_provider.h"
#include "providers/provider_factory.h"

#include <future>
#include <iostream>
#include <stop_token>

namespace
{

void print_usage()
{
    std::cerr << "Usage: echoes-cli login onedrive\n";
}

} // namespace

int handle_login_command(const Args& args)
{
    if (!args.flags.empty() || args.positional.size() != 2)
    {
        print_usage();
        return 1;
    }

    if (args.positional[1] != "onedrive")
    {
        std::cerr << "Unsupported provider: " << args.positional[1] << '\n';
        return 1;
    }

    OpenSSLCryptoProvider crypto_provider;
    InMemoryAuthStore auth_store;
    auto auth_provider = providers::create_auth_provider(entities::ProviderId::OneDrive,
                                                         providers::GlobalDependencies{
                                                             .crypto_provider = &crypto_provider,
                                                             .auth_store = &auth_store,
                                                         });

    std::promise<AuthConnectResult> completion_promise;
    auto completion_future = completion_promise.get_future();
    std::stop_source cancellation;

    const auto start_url =
        auth_provider->connect(cancellation.get_token(),
                               [&](AuthConnectResult result)
                               {
                                   completion_promise.set_value(std::move(result));
                                   cancellation.request_stop();
                               });

    std::cout << "Open this URL in your browser to sign in:\n" << start_url << '\n';

    const auto result = completion_future.get();

    if (result.status == AuthConnectStatus::Ok)
    {
        std::cout << "Login completed.\n";
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
