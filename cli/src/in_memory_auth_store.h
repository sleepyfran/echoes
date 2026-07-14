#pragma once

#include "auth_store.h"

#include <map>
#include <mutex>

class InMemoryAuthStore final : public AuthStore
{
  public:
    bool save(const entities::AuthInfo& info) const override;
    [[nodiscard]] std::optional<entities::AuthInfo>
    retrieve(entities::ProviderId provider_id) const override;

  private:
    mutable std::mutex mutex;
    mutable std::map<entities::ProviderId, entities::AuthInfo> auth_info_by_provider;
};
