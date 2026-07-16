#include "providers/provider_factory.h"
#include "entities/provider.h"
#include "onedrive/onedrive_provider.h"
#include "providers/auth_provider.h"
#include "providers/onedrive_config.h"
#include <memory>
#include <optional>

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
            deps.auth_store);
    }
    default:
        std::abort();
    }
}

std::optional<std::unique_ptr<media_provider::FileBasedProvider>>
create_file_based_provider(entities::ProviderId provider_id, GlobalDependencies deps)
{
    switch (provider_id)
    {
    case entities::ProviderId::OneDrive:
    {
        return std::make_unique<media_provider::OneDriveMediaProvider>(deps.auth_store);
    }
    default:
    {
        return std::nullopt;
    }
    }
}
} // namespace providers
