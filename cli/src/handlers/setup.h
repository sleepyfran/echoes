#pragma once

#include "args.h"
#include "auth_store.h"

/**
 * Attempts to begin the setup of a file-based provider. If not logged in, it shows an error
 * indicating that the user needs to first log-in. If the provider is not a file-based one, then it
 * will also show an error.
 */
int handle_setup(AuthStore& auth_store, const Args& args);
