#include "queries/auth_queries.h"
#include <ctime>
#include <format>
#include <optional>

namespace queries::auth
{
bool is_auth_info_valid(const entities::AuthInfo& auth_info)
{
    const auto current_time = std::time(nullptr);
    return auth_info.expires_on > current_time;
}

bool is_auth_info_valid(const std::optional<entities::AuthInfo>& opt_auth_info)
{
    if (!opt_auth_info)
    {
        return false;
    }

    return is_auth_info_valid(opt_auth_info.value());
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
