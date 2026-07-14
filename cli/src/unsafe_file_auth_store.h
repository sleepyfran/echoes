#pragma once

#include "auth_store.h"
#include "entities/provider.h"

#include <map>
#include <mutex>

class UnsafeFileAuthStore final : public AuthStore
{
    mutable std::mutex mutex;
    mutable std::map<entities::ProviderId, entities::AuthInfo> auth_info_by_provider;

  public:
    void load_from_file();
    bool save(const entities::AuthInfo& info) const override;
    [[nodiscard]] std::optional<entities::AuthInfo>
    retrieve(entities::ProviderId provider_id) const override;
};
