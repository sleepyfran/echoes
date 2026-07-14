#include "in_memory_auth_store.h"

bool InMemoryAuthStore::save(const entities::AuthInfo& info) const
{
    const std::lock_guard lock(mutex);
    auth_info_by_provider.insert_or_assign(info.provider_id, info);
    return true;
}

std::optional<entities::AuthInfo>
InMemoryAuthStore::retrieve(entities::ProviderId provider_id) const
{
    const std::lock_guard lock(mutex);
    const auto found = auth_info_by_provider.find(provider_id);
    if (found == auth_info_by_provider.end())
    {
        return std::nullopt;
    }

    return found->second;
}
