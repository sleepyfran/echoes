#include "onedrive/onedrive_provider.h"
#include "providers/auth_provider.h"
#include "providers/provider_factory.h"
#include <memory>
#include <type_traits>

namespace providers
{
std::unique_ptr<AuthProvider> create_auth_provider(entities::ProviderId id, GlobalDependencies deps,
                                                   AuthProviderArgs& args)
{
    switch (id)
    {
    case entities::ProviderId::OneDrive:
    {
        return std::visit(
            [id, deps = std::move(deps)](auto&& arg) mutable -> std::unique_ptr<AuthProvider>
            {
                using T = std::decay_t<decltype(arg)>;
                if constexpr (std::is_same_v<T, OneDriveParams>)
                {
                    return std::make_unique<OneDriveAuthProvider>(arg.config,
                                                                  std::move(deps.crypto_provider));
                }
                
                return nullptr;
            },
            args);
    }
    default:
        std::abort();
    }
}
} // namespace providers
