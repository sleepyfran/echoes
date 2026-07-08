#pragma once

#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace utils
{
constexpr const char* content_type_text = "text/plain";

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
} // namespace utils
