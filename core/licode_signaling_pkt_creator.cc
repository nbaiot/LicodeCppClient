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
  stream["offerFromErizo"] = true;
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

std::string LicodeSignalingPktCreator::CreateConnectionAnswerMsg(const std::string& sdp, int maxVideoBW) {
  nlohmann::json msg;
  msg["type"] = "answer";
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
  msg["type"] = "candidate";
  nlohmann::json cand;
  cand["sdpMLineIndex"] = sdpMLineIndex;
  cand["sdpMid"] = sdpMid;
  cand["candidate"] = candidate;
  msg["candidate"] = cand;
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
  /// TODO: fixme
  connection["browser"] = "cpp";

  connectionJson[1] = connection;
  connectionJson[2] = nlohmann::json::value_t::null;
  return connectionJson.dump();
}

std::string
LicodeSignalingPktCreator::CreateOfferOrAnswerPkt(bool offer, const std::string& erizoId, const std::string& connId,
                                                  const std::string& sdp, int maxVideoBW) {
  auto json = nlohmann::json::array();
  json[0] = "connectionMessage";

  nlohmann::json info;
  /// TODO:fixme
  info["browser"] = "cpp";
  info["connectionId"] = connId;
  info["erizoId"] = erizoId;

  nlohmann::json msg;
  if (offer) {
    msg = nlohmann::json::parse(CreateConnectionOfferMsg(sdp, maxVideoBW));
  } else {
    msg = nlohmann::json::parse(CreateConnectionAnswerMsg(sdp, maxVideoBW));
  }
  info["msg"] = msg;
  json[1] = info;

  json[2] = nlohmann::json::value_t::null;
  return json.dump();
}


}