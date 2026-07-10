#include "providers/provider_factory.h"
#include "onedrive/onedrive_provider.h"
#include "providers/auth_provider.h"
#include "providers/onedrive_config.h"
#include <memory>

namespace providers
{
std::unique_ptr<AuthProvider> create_auth_provider(entities::ProviderId id, GlobalDependencies deps)
{
    switch (id)
    {
    case entities::ProviderId::OneDrive:
    {
        return std::make_unique<OneDriveAuthProvider>(
            OneDriveConfig{
                .client_id = ONEDRIVE_AUTH_CLIENT_ID,
            },
            std::move(deps.crypto_provider));
    }
    default:
        std::abort();
    }
}
} // namespace providers
