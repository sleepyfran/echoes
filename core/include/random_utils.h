#pragma once

#include <string>

namespace random_utils
{
/**
 * Generates a random string of the given length using the RFC 7636 character set.
 */
std::string generate_random_string(std::size_t size);
} // namespace random_utils
