//
// Created by nbaiot@126.com on 2020/9/25.
//

#include "licode_signaling_pkt_parser.h"

#include <nlohmann/json.hpp>
#include <glog/logging.h>

namespace nbaiot {

void LicodeSignalingPktParser::ParseOpenMsg(const std::string& msg, std::string& sid, int32_t& pingInterval,
                                            int32_t& pingTimeout) {
  try {
    auto openMsg = nlohmann::json::parse(msg);
    sid = openMsg.value<std::string>("sid", "");
    pingInterval = openMsg.value<int32_t>("pingInterval", 25000);
    pingTimeout = openMsg.value<int32_t>("pingTimeout", 60000);
  } catch (const std::exception& e) {
    LOG(ERROR) << ">>>>>>>>>>LicodeSignalingPktParser ParseOpenMsg error:" << e.what();
  }

}

}