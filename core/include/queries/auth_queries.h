#pragma once

#include "auth_store.h"
#include "entities/auth.h"
#include "entities/provider.h"
#include "httplib.hpp"
namespace queries::auth
{
using Headers = httplib::Headers;

/**
 * Checks if the given auth info is still valid based on the expires on field.
 */
bool is_auth_info_valid(const entities::AuthInfo& auth_info);

/**
 * Creates the authorization headers needed to authorize a request with the auth info of the given
 * provider. Returns none if there's no auth known for this provider.
 */
std::optional<Headers> authorized_headers(const AuthStore* auth_store,
                                          entities::ProviderId provider_id);
} // namespace queries::auth
