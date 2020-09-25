//
// Created by nbaiot@126.com on 2020/9/25.
//

#include "licode_signaling_pkt_creator.h"

#include <nlohmann/json.hpp>

namespace nbaiot {

std::string
LicodeSignalingPktCreator::CreateTokenPkt(bool singlePC, const LicodeToken& token) {
  nlohmann::json tokenJson;
  tokenJson["tokenId"] = token.tokenId;
  tokenJson["host"] = token.host;
  tokenJson["signature"] = token.signature;
  tokenJson["secure"] = token.secure;

  nlohmann::json data;
  data["singlePC"] = singlePC;
  data["token"] = tokenJson;

  auto msg = nlohmann::json::array({"token", data});

  return msg.dump();
}
}