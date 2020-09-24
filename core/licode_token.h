//
// Created by cuangenglei-os@360os.com on 2020/9/24.
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

  bool Isvalid() {
    return !tokenId.empty() && !host.empty() && !signature.empty();
  }
};

}

#endif //LICODECPPCLIENT_LICODE_TOKEN_H
