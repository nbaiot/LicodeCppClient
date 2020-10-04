//
// Created by nbaiot@126.com on 2020/10/4.
//

#ifndef LICODECPPCLIENT_ICE_SERVER_H
#define LICODECPPCLIENT_ICE_SERVER_H

#include <string>

namespace nbaiot {

struct IceServer {
  std::string url;
  std::string username;
  std::string credential;

  bool operator==(const IceServer& rhs) const {
    return url == rhs.url &&
           username == rhs.username &&
           credential == rhs.credential;
  }

  bool operator!=(const IceServer& rhs) const {
    return !(rhs == *this);
  }
};

}

#endif //LICODECPPCLIENT_ICE_SERVER_H
