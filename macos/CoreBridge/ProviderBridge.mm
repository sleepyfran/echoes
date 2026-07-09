#include "MacOsCryptoProvider.h"
#include "ProviderBridge.h"
#include "entities/provider.h"
#include "providers/provider_factory.h"
#include <memory>
#include <stop_token>
#include <string>

// Keeps the in-flight auth provider alive for the duration of the authentication flow. Without
// this it would be destroyed as soon as `start_authentication:` returns, tearing down the
// listening server while the background worker thread that owns it is still running.
static std::unique_ptr<AuthProvider> s_active_auth_provider;
static std::stop_source s_active_auth_stop_source;

static entities::ProviderId ToProviderId(ProviderIdBridge provider_id) {
    switch (provider_id) {
        case ProviderIdBridgeOneDrive:
            return entities::ProviderId::OneDrive;
        case ProviderIdBridgeSpotify:
            return entities::ProviderId::Spotify;
    }
}

// TODO: source these from a real config (secrets store / build config) instead of hardcoding
// placeholders once the OneDrive app registration details are available.
static OneDriveConfig MakeOneDriveConfig() {
    return OneDriveConfig{
        .base_url = "https://login.microsoftonline.com",
        .token_req_endpoint = "/common/oauth2/v2.0/token",
        .start_endpoint = "/common/oauth2/v2.0/authorize",
        .port = 443,
        .listening_server_port = 8080,
        .client_id = "",
    };
}

@implementation ProviderBridge

+ (NSString *)start_authentication:(ProviderIdBridge)provider_id
                         completion:(ProviderAuthCompletion)completion {
    providers::GlobalDependencies deps{
        .crypto_provider = std::make_unique<MacOsCryptoProvider>(),
    };
    providers::AuthProviderArgs args = providers::OneDriveParams{MakeOneDriveConfig()};

    s_active_auth_provider =
        providers::create_auth_provider(ToProviderId(provider_id), std::move(deps), args);
    s_active_auth_stop_source = std::stop_source();

    // Blocks must be copied to escape the scope they were declared in.
    ProviderAuthCompletion completion_copy = [completion copy];

    auto start_url = s_active_auth_provider->connect(
        s_active_auth_stop_source.get_token(),
        [completion_copy](AuthConnectResult result) {
            BOOL success = result.status == AuthConnectStatus::Ok;
            NSString* error_message = result.error_msg
                ? [NSString stringWithUTF8String:result.error_msg->c_str()]
                : nil;

            // The callback can fire from the httplib server thread; hop back to the main
            // queue so observers on the Swift side can safely update UI state.
            dispatch_async(dispatch_get_main_queue(), ^{
                completion_copy(success, error_message);
            });
        });

    return [NSString stringWithUTF8String:start_url.c_str()];
}

@end
