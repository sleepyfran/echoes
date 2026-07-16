#include "queries/auth_queries.h"
#include <ctime>
#include <optional>

namespace queries::auth
{
bool is_auth_info_valid(const entities::AuthInfo& auth_info)
{
    const auto current_time = std::time(nullptr);
    return auth_info.expires_on > current_time;
}

std::optional<Headers> authorized_headers(const AuthStore* auth_store,
                                          entities::ProviderId provider_id)
{
    const auto auth_info = auth_store->retrieve(provider_id);
    if (!auth_info || !is_auth_info_valid(auth_info.value()))
    {
        return std::nullopt;
    }

    return Headers{{"Authorization", std::format("Bearer {}", auth_info.value().access_token)}};
}
} // namespace queries::auth
