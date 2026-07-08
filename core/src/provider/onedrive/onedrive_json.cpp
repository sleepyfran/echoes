#include "onedrive_json.h"
#include "entities/auth.h"
#include "json.hpp"

namespace onedrive
{
std::optional<entities::AuthInfo> parse_auth_info(std::string& response)
{
    using json = nlohmann::json;

    try
    {
        auto body = json::parse(response);
        auto access_token = body.at("access_token").get<std::string>();
        auto expires_in = body.value("expires_in", 3600);
        auto expires_on = std::time(nullptr) + expires_in;

        return entities::AuthInfo{
            .provider_id = entities::ProviderId::OneDrive,
            .access_token = access_token,
            .provider_specific_auth_info =
                entities::MsalSpecificAuthInfo{
                    .home_account_id = "",
                    .environment = "login.microsoftonline.com",
                    .tenant_id = "",
                    .username = "",
                    .local_account_id = "",
                },
            .expires_on = expires_on,
        };
    }
    catch (json::parse_error& e)
    {
        return std::nullopt;
    }
    catch (json::exception& e)
    {
        return std::nullopt;
    }
}
} // namespace onedrive
