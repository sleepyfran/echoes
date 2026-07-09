#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace crypto
{
class CryptoProvider
{
  public:
    virtual ~CryptoProvider() = default;

    /**
     * Generates the SHA256 of the given input using the operating system's
     * crypto library.
     */
    virtual std::vector<uint8_t> sha256(std::string_view input) = 0;
};
} // namespace crypto
