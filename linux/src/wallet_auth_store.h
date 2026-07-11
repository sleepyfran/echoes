#include "auth_store.h"
#include <KWallet>
#include <qwindowdefs.h>

namespace wallet
{
constexpr const char* wallet_name = "me.sleepyfran.echoes";

class WalletAuthStore : public AuthStore
{
    WId window_id;

  public:
    bool save(const entities::AuthInfo& info) const override;
    [[nodiscard]] std::optional<entities::AuthInfo>
    retrieve(entities::ProviderId provider_id) const override;
    void set_window_id(WId window_id);
};
} // namespace wallet
