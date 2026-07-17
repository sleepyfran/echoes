#pragma once

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

namespace utils
{
constexpr const char* content_type_text = "text/plain";
constexpr const char* content_type_json = "application/json";

/**
 * @brief Helper utility for std::visit to allow "pattern matching" with multiple lambdas.
 *
 * This struct uses variadic inheritance to merge multiple callable objects (usually lambdas)
 * into a single object. This allows std::visit to dispatch the variant's active type to
 * the corresponding lambda overload.
 *
 * Example Usage:
 * @code
 * std::variant<int, std::string> v = "Hello";
 * std::visit(overloaded {
 *     [](int val) { std::cout << "Integer: " << val; },
 *     [](const std::string& val) { std::cout << "String: " << val; }
 * }, v);
 * @endcode
 */
template <class... Ts> struct overloaded : Ts...
{
    using Ts::operator()...;
};
template <class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

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
 * Converts regular base64 output to the unpadded base64url format used by PKCE.
 */
inline std::string base64_url_encode(std::string encoded)
{
    std::ranges::replace(encoded, '+', '-');
    std::ranges::replace(encoded, '/', '_');

    while (!encoded.empty() && encoded.back() == '=')
    {
        encoded.pop_back();
    }

    return encoded;
}

/**
 * Creates a string from the given vector of unsigned integers.
 */
inline std::string vector_to_str(std::vector<uint8_t>& vec)
{
    return {vec.begin(), vec.end()};
}
} // namespace utils
