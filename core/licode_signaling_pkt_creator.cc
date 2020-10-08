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

std::string LicodeSignalingPktCreator::CreateSubscribeStreamPkt(uint64_t streamId) {
  auto subscribeJson = nlohmann::json::array();
  subscribeJson[0] = "subscribe";

  nlohmann::json stream;
  stream["streamId"] = streamId;
  stream["audio"] = true;
  stream["video"] = true;
  stream["data"] = true;
  stream["maxVideoBW"] = 300;
  stream["offerFromErizo"] = false;
  /// TODO: fixme
  stream["browser"] = "cpp";

  nlohmann::json metadata;
  metadata["type"] = "subscriber";
  stream["metadata"] = metadata;

  nlohmann::json muteStream;
  muteStream["audio"] = false;
  muteStream["video"] = false;
  stream["muteStream"] = muteStream;

  stream["slideShowMode"] = false;

  subscribeJson[1] = stream;
  subscribeJson[2] = nlohmann::json::value_t::null;

  return subscribeJson.dump();
}

std::string LicodeSignalingPktCreator::CreateConnectionOfferMsg(const std::string& sdp, int maxVideoBW) {
  nlohmann::json msg;
  msg["type"] = "offer";
  msg["sdp"] = sdp;
  msg["receivedSessionVersion"] = -1;
  nlohmann::json config;
  config["maxVideoBW"] = maxVideoBW;
  msg["config"] = config;
  return msg.dump();
}

std::string LicodeSignalingPktCreator::CreateConnectionCandidateMsg(int sdpMLineIndex, const std::string& sdpMid,
                                                                    const std::string& candidate) {
  nlohmann::json msg;
  msg["sdpMLineIndex"] = sdpMLineIndex;
  msg["sdpMid"] = sdpMid;
  msg["candidate"] = candidate;
  msg["receivedSessionVersion"] = 0;
  return msg.dump();
}

std::string LicodeSignalingPktCreator::CreateConnectionPtk(const std::string& connectionId,
                                                           const std::string& erizoId,
                                                           const std::string& msg) {
  auto connectionJson = nlohmann::json::array();
  connectionJson[0] = "connectionMessage";

  nlohmann::json connection;
  connection["connectionId"] = connectionId;
  connection["erizoId"] = erizoId;
  connection["msg"] = nlohmann::json::parse(msg);
  connection["browser"] = "cpp";

  connectionJson[1] = connection;
  connectionJson[2] = nlohmann::json::value_t::null;
  return connectionJson.dump();
}
}