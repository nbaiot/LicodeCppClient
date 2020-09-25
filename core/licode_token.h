//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_TOKEN_H
#define LICODECPPCLIENT_LICODE_TOKEN_H

#include <string>

namespace nbaiot {

struct LicodeToken {
  std::string tokenId;
  std::string host;
  bool secure;
  std::string signature;

  bool operator==(const LicodeToken& rhs) const {
    return tokenId == rhs.tokenId &&
           host == rhs.host &&
           secure == rhs.secure &&
           signature == rhs.signature;
  }

  bool operator!=(const LicodeToken& rhs) const {
    return !(rhs == *this);
  }

  bool Isvalid() {
    return !tokenId.empty() && !host.empty() && !signature.empty();
  }
};

}

#endif //LICODECPPCLIENT_LICODE_TOKEN_H
