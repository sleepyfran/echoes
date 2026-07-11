#include "wallet_auth_store.h"
#include "entities/auth.h"
#include "entities/provider.h"
#include <kwallet.h>
#include <qlogging.h>
#include <qmap.h>
#include <qobject.h>
#include <type_traits>
#include <variant>

namespace wallet
{
QString namespaced_key(entities::ProviderId provider_id, std::string key)
{
    std::string provider_name;
    switch (provider_id)
    {
    case entities::ProviderId::OneDrive:
        provider_name = "onedrive";
        break;
    case entities::ProviderId::Spotify:
        provider_name = "spotify";
        break;
    default:
        break;
    }

    return QString::fromUtf8(std::format("{}/{}", provider_name, key));
}

void populate_item(QMap<QString, QString>& map, const std::string& key, const std::string& value)
{
    map[QString::fromStdString(key)] = QString::fromStdString(value);
}

QMap<QString, QString> sealize_provider_info(const entities::AuthInfo& info)
{
    QMap<QString, QString> data;

    std::visit(
        [&](const auto& provider_info)
        {
            using T = std::decay_t<decltype(provider_info)>;

            if constexpr (std::is_same_v<T, entities::MsalSpecificAuthInfo>)
            {
                populate_item(data, "home_account_id", provider_info.home_account_id);
                populate_item(data, "environment", provider_info.environment);
                populate_item(data, "tenant_id", provider_info.tenant_id);
                populate_item(data, "username", provider_info.username);
                populate_item(data, "local_account_id", provider_info.local_account_id);
            }
            else if constexpr (std::is_same_v<T, entities::SpotifySpecificAuthInfo>)
            {
                populate_item(data, "type", "spotify");
                populate_item(data, "refresh_token", provider_info.refresh_token);
            }
        },
        info.provider_specific_auth_info);

    return data;
}

bool WalletAuthStore::save(const entities::AuthInfo& info) const
{
    auto* wallet = KWallet::Wallet::openWallet(KWallet::Wallet::NetworkWallet(), window_id);
    if (!wallet)
    {
        qWarning("Unable to open network wallet, returned nullptr");
        return false;
    }

    const auto folder_name = QString::fromUtf8(wallet_name);
    if (!wallet->createFolder(folder_name))
    {
        qWarning("Unable to create folder");
        return false;
    }

    auto folder_exists = wallet->setFolder(folder_name);
    auto token_write_status = wallet->writePassword(
        namespaced_key(info.provider_id, "access_token"), QString::fromUtf8(info.access_token));
    auto info_write_status =
        wallet->writeMap(namespaced_key(info.provider_id, "extra"), sealize_provider_info(info));

    return folder_exists && token_write_status == 0 && info_write_status == 0;
}

std::optional<entities::AuthInfo> WalletAuthStore::retrieve(entities::ProviderId provider_id) const
{
    // TODO: Implement.
    return std::nullopt;
};
} // namespace wallet
