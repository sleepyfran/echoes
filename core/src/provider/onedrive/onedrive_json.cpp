#include "onedrive_json.h"
#include "entities/auth.h"
#include "entities/file_system.h"
#include "json.hpp"
#include "providers/media_provider.h"
#include <cstdint>

namespace onedrive
{
using json = nlohmann::json;

std::optional<entities::AuthInfo> parse_auth_info(std::string& response)
{

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

std::optional<media_provider::FolderContent> parse_list_response(std::string& response)
{
    try
    {
        auto body = json::parse(response);
        media_provider::FolderContent content;
        for (const auto& v : body.at("value"))
        {
            const auto id = v.value("id", "");
            const auto name = v.value("name", "");
            const uint64_t size = v.value("size", 0);

            if (v.contains("folder"))
            {
                const auto& f = v.at("folder");
                const uint32_t child_count = f.value("child_count", 0);
                content.emplace_back(entities::FolderMetadata{
                    {.id = entities::ItemId{id}, .name = name, .byte_size = size},
                    .child_count = 0,
                });
            }
            else if (v.contains("file"))
            {
                const auto& f = v.at("file");
                const auto mime_type = f.value("mime_type", "");
                const auto download_url = f.value("@microsoft.graph.downloadUrl", "");
                content.emplace_back(entities::FileMetadata{
                    {.id = entities::ItemId{id}, .name = name, .byte_size = size},
                    .mime_type = mime_type,
                    .download_url = download_url});
            }
        }

        return content;
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
