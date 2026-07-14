#pragma once

#include <string_view>
#include <vector>

struct Args
{
    /**
     * Includes the list of positional arguments that were given. Example:
     * `echoes-cli login onedrive`
     *            ^ positional ^
     */
    std::vector<std::string_view> positional;

    /**
     * Includes the list of flags that were given, without their prefix `-` or `--`. Example:
     * `echoes-cli login onedrive -f --something`
     *                            ^ flags ^
     * In the vector, those would be included as `f` and `something`.
     */
    std::vector<std::string_view> flags;
};
