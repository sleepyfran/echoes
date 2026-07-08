#pragma once

#include "entities/auth.h"
#include <optional>

namespace onedrive
{
/**
 * Attempts to parse the response of a request to exchange an authentication code returned by the
 * login flow into tokens.
 */
std::optional<entities::AuthInfo> parse_auth_info(std::string& response);
} // namespace onedrive
