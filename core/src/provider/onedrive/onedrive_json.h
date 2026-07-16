#pragma once

#include "entities/auth.h"
#include "providers/media_provider.h"
#include <optional>

namespace onedrive
{
/**
 * Attempts to parse the response of a request to exchange an authentication code returned by the
 * login flow into tokens.
 */
std::optional<entities::AuthInfo> parse_auth_info(std::string& response);

/**
 * Attempts to parse the response of a listing request that returns
 */
std::optional<media_provider::FolderContent> parse_list_response(std::string& response);
} // namespace onedrive
