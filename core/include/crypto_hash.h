#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace crypto
{
[[nodiscard]] std::vector<uint8_t> sha256(std::string_view input);
} // namespace crypto
