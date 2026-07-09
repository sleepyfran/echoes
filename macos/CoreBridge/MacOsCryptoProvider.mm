#include "MacOsCryptoProvider.h"
#include <CommonCrypto/CommonDigest.h>

std::vector<uint8_t> MacOsCryptoProvider::sha256(std::string_view input)
{
    std::vector<uint8_t> hash(CC_SHA256_DIGEST_LENGTH);
    CC_SHA256(input.data(), (CC_LONG)input.size(), hash.data());
    return hash;
}
