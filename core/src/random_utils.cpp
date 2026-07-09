#include "random.h"

#include <algorithm>
#include <random>

namespace random_utils
{
/**
 * Implementation from Harris Brakmić's Keybloack PKCE:
 * https://github.com/brakmic/keycloak-pkce-cpp/blob/main/lib/src/pkce/pkce.hpp
 */
std::string generate_random_string(std::size_t size)
{
    static constexpr std::string_view charset =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";

    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, charset.size() - 1);

    std::string result;
    result.reserve(size);
    std::generate_n(std::back_inserter(result), size, [&] { return charset[dist(gen)]; });

    return result;
}
} // namespace random_utils
