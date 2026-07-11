#pragma once

#include "crypto_provider.h"

class OpenSSLCryptoProvider : public crypto::CryptoProvider
{
  public:
    [[nodiscard]] std::vector<uint8_t> sha256(std::string_view input) const override;
};
