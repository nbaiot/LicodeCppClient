//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_TOKEN_CREATOR_H
#define LICODECPPCLIENT_LICODE_TOKEN_CREATOR_H

#include "licode_token.h"

namespace nbaiot {

class LicodeTokenCreator {
public:
  static LicodeToken SyncCreate(const std::string& url, const std::string& userName, int timeoutMS);
};

}

#endif //LICODECPPCLIENT_LICODE_TOKEN_CREATOR_H
