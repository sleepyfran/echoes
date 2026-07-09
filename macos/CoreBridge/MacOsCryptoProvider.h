#pragma once

#include "crypto_provider.h"

class MacOsCryptoProvider : public crypto::CryptoProvider
{
  public:
    std::vector<uint8_t> sha256(std::string_view input) override;
};
