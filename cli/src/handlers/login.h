#pragma once

#include "args.h"
#include "auth_store.h"

/**
 * Attempts to start the login process for the provider specified in the arguments.
 */
int handle_login_command(AuthStore& store, const Args& args);
