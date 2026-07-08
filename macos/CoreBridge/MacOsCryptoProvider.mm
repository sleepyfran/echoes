#include "crypto_provider.h"
#include <CommonCrypto/CommonDigest.h>

class MacOsCryptoProvider : public CryptoProvider
{
public:
    std::vector<uint8_t> sha256(std::string_view input) override
    {
        std::vector<uint8_t> hash(CC_SHA256_DIGEST_LENGTH);
        CC_SHA256(input.data(), (CC_LONG)input.size(), hash.data());
        return hash;
    }
}
