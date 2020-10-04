//
// Created by nbaiot@126.com on 2020/10/2.
//

#include "hmac.h"

#include <nettle/hmac.h>
#include <boost/algorithm/hex.hpp>

namespace nbaiot {

class HmacImpl {

public:
  explicit HmacImpl(const std::string& key) {
    hmac_sha1_set_key(&ctx_, key.length(), reinterpret_cast<const uint8_t*>(key.c_str()));
  }

  ~HmacImpl() = default;

  std::string Sha1(const std::string& str) {
    hmac_sha1_update(&ctx_, str.length(), reinterpret_cast<const uint8_t*>(str.c_str()));
    uint8_t digest[SHA1_DIGEST_SIZE + 1] = {0};
    hmac_sha1_digest(&ctx_, SHA1_DIGEST_SIZE, digest);
    std::string out;
    boost::algorithm::hex_lower(digest, std::back_inserter(out));
    return out;
  }

private:
  hmac_sha1_ctx ctx_{};

};


Hmac::Hmac(const std::string& key) {
  impl_ = std::make_unique<HmacImpl>(key);
}

std::string Hmac::Sha1(const std::string& str) {
  return impl_->Sha1(str);
}

Hmac::~Hmac() = default;

};