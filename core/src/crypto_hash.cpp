#include "crypto_hash.h"

#include "picosha2.h"

namespace crypto
{
std::vector<uint8_t> sha256(std::string_view input)
{
    std::vector<uint8_t> hash(picosha2::k_digest_size);
    picosha2::hash256(input.begin(), input.end(), hash.begin(), hash.end());
    return hash;
}
} // namespace crypto
