#pragma once

#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace utils
{
constexpr const char* content_type_text = "text/plain";

/**
 * Encodes a URL to make it safe to be used in URLs per the RFC 3986 standard.
 */
inline std::string url_encode(std::string_view value)
{
    std::ostringstream encoded;
    encoded << std::uppercase << std::hex;

    for (auto ch : value)
    {
        auto byte = static_cast<unsigned char>(ch);
        if ((std::isalnum(byte) != 0) || byte == '-' || byte == '_' || byte == '.' || byte == '~')
        {
            encoded << ch;
            continue;
        }

        encoded << '%' << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
    }

    return encoded.str();
}

/**
 * Creates a string from the given vector of unsigned integers.
 */
inline std::string vector_to_str(std::vector<uint8_t>& vec)
{
    return {vec.begin(), vec.end()};
}
} // namespace utils
