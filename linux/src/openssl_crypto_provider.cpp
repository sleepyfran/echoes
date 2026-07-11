#include "openssl_crypto_provider.h"

#include <openssl/sha.h>

std::vector<uint8_t> OpenSSLCryptoProvider::sha256(std::string_view input) const
{
    std::vector<uint8_t> hash(SHA256_DIGEST_LENGTH);
    SHA256(reinterpret_cast<const unsigned char*>(input.data()), input.size(), hash.data());
    return hash;
}
