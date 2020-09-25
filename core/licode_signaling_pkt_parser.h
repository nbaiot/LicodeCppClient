//
// Created by nbaiot@126.com on 2020/9/25.
//

#ifndef LICODECPPCLIENT_LICODE_SIGNALING_PKT_PARSER_H
#define LICODECPPCLIENT_LICODE_SIGNALING_PKT_PARSER_H

#include <string>
#include <cstdint>

namespace nbaiot {

class LicodeSignalingPktParser {

public:
  static void ParseOpenMsg(const std::string& msg,
                           std::string& sid, int32_t& pingInterval, int32_t& pingTimeout);

};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_PKT_PARSER_H
