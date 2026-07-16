#include "entities/provider.h"
#include "httplib.hpp"
#include "onedrive_json.h"
#include "onedrive_provider.h"
#include "providers/media_provider.h"
#include "providers/onedrive_config.h"
#include "queries/auth_queries.h"
#include "utils.h"

namespace media_provider
{
OneDriveMediaProvider::OneDriveMediaProvider(AuthStore* auth_store)
    : client{onedrive_graph_host, onedrive_port},
      base_headers({{"Accept", utils::content_type_json}}), auth_store{auth_store}
{
}

FolderContentResult OneDriveMediaProvider::list_root()
{
    auto auth_headers =
        queries::auth::authorized_headers(auth_store, entities::ProviderId::OneDrive);
    if (!auth_headers)
    {
        return {.status = MediaProviderResultStatus::MissingAuth};
    }

    auto headers = auth_headers.value();
    headers.insert(base_headers.begin(), base_headers.end());
    auto res = client.Get(onedrive_list_root_endpoint, headers);
    if (!res || res->status < 200 || res->status >= 300)
    {
        // TODO: Actually type this based on errors.
        return {.status = MediaProviderResultStatus::ApiError,
                .error = MediaProviderError::Unknown};
    }

    const auto parsed_response = onedrive::parse_list_response(res->body);
    if (!parsed_response)
    {
        // TODO: Actually type this based on errors.
        return {.status = MediaProviderResultStatus::ApiError,
                .error = MediaProviderError::Unknown};
    }

    return {.status = MediaProviderResultStatus::Ok, .result = parsed_response.value()};
}

FolderContentResult OneDriveMediaProvider::list_folder(entities::FolderMetadata& folder)
{
    return {};
};

UrlResult OneDriveMediaProvider::file_url_by_id(entities::ItemId& id)
{
    return {};
};
} // namespace media_provider
