//
// Created by nbaiot@126.com on 2020/9/25.
//

#ifndef LICODECPPCLIENT_LICODE_SIGNALING_PKT_CREATOR_H
#define LICODECPPCLIENT_LICODE_SIGNALING_PKT_CREATOR_H

#include <string>
#include "licode_token.h"

namespace nbaiot {

class LicodeSignalingPktCreator {

public:
  static std::string CreateTokenPkt(bool singlePC, const LicodeToken& token);

};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_PKT_CREATOR_H
