//
// Created by nbaiot@126.com on 2020/9/25.
//

#include "licode_signaling_pkt_builder.h"

#include <nlohmann/json.hpp>

namespace nbaiot {

PublicStreamPktBuilder& PublicStreamPktBuilder::SetLabel(std::string label) {
  label_ = std::move(label);
  return *this;
}

PublicStreamPktBuilder& PublicStreamPktBuilder::SetState(std::string state) {
  state_ = std::move(state);
  return *this;
}

std::string PublicStreamPktBuilder::Build() {
  auto publishJson = nlohmann::json::array();
  publishJson[0] = "publish";

  nlohmann::json config;
  config["audio"] = audio_;
  config["video"] = video_;
  config["data"] = data_;
  config["label"] = label_;
  config["screen"] = screen_;
  config["minVideoBW"] = min_video_bw_;
  config["maxVideoBW"] = max_video_bw_;
  config["attributes"] = nlohmann::json::object();

  nlohmann::json metadata;
  metadata["type"] = "publisher";
  config["metadata"] = metadata;

  nlohmann::json muteStream;
  muteStream["audio"] = mute_audio_;
  muteStream["video"] = mute_video_;
  config["muteStream"] = muteStream;


  publishJson[1] = config;
  publishJson[2] = nlohmann::json::value_t::null;

  return publishJson.dump();
}

updateStreamAttributesPktBuilder& updateStreamAttributesPktBuilder::SetStreamId(uint64_t streamId) {
  stream_id_ = streamId;
  return *this;
}

updateStreamAttributesPktBuilder& updateStreamAttributesPktBuilder::SetTypeAttr(std::string type) {
  type_attr_ = std::move(type);
  return *this;
}

std::string updateStreamAttributesPktBuilder::Build() {
  auto updateAttrJson = nlohmann::json::array();
  updateAttrJson[0] = "updateStreamAttributes";

  nlohmann::json config;
  config["id"] = stream_id_;

  nlohmann::json attrs;
  attrs["type"] = type_attr_;
  config["attrs"] = attrs;

  updateAttrJson[1] = config;

  return updateAttrJson.dump();
}

SubscribeStreamPktBuilder& SubscribeStreamPktBuilder::SetStreamId(uint64_t streamId) {
  stream_id_ = streamId;
  return *this;
}

SubscribeStreamPktBuilder& SubscribeStreamPktBuilder::SetOfferFromErizo(bool enable) {
  offer_from_erizo_ = enable;
  return *this;
}

std::string SubscribeStreamPktBuilder::Build() {
  auto subscribeJson = nlohmann::json::array();
  subscribeJson[0] = "subscribe";

  nlohmann::json stream;
  stream["streamId"] = stream_id_;
  stream["audio"] = audio_;
  stream["video"] = video_;
  stream["data"] = data_;
  stream["maxVideoBW"] = max_video_bw_;
  stream["offerFromErizo"] = offer_from_erizo_;
  stream["browser"] = browser_;

  nlohmann::json metadata;
  metadata["type"] = "subscriber";
  stream["metadata"] = metadata;

  nlohmann::json muteStream;
  muteStream["audio"] = mute_audio_;
  muteStream["video"] = mute_video_;
  stream["muteStream"] = muteStream;

  stream["slideShowMode"] = false;

  subscribeJson[1] = stream;
  subscribeJson[2] = nlohmann::json::value_t::null;

  return subscribeJson.dump();
}

UnsubscribeStreamPktBuilder& UnsubscribeStreamPktBuilder::SetStreamId(uint64_t streamId) {
  stream_id_ = streamId;
  return *this;
}

std::string UnsubscribeStreamPktBuilder::Build() {
  auto unsubscribeJson = nlohmann::json::array();
  unsubscribeJson[0] = "unsubscribe";
  unsubscribeJson[1] = stream_id_;
  unsubscribeJson[2] = nlohmann::json::value_t::null;
  return unsubscribeJson.dump();
}

OfferOrAnswerPktBuilder& OfferOrAnswerPktBuilder::SetErizoId(std::string id) {
  erizo_id_ = std::move(id);
  return *this;
}

OfferOrAnswerPktBuilder& OfferOrAnswerPktBuilder::SetConnId(std::string id) {
  connection_id_ = std::move(id);
  return *this;
}

OfferOrAnswerPktBuilder& OfferOrAnswerPktBuilder::SetType(std::string type) {
  type_ = std::move(type);
  return *this;
}

OfferOrAnswerPktBuilder& OfferOrAnswerPktBuilder::SetSdp(std::string sdp) {
  sdp_ = std::move(sdp);
  return *this;
}

std::string OfferOrAnswerPktBuilder::Build() {
  auto json = nlohmann::json::array();
  json[0] = "connectionMessage";

  nlohmann::json info;
  /// TODO:fixme
  info["browser"] = browser_;
  info["connectionId"] = connection_id_;
  info["erizoId"] = erizo_id_;

  nlohmann::json msg;
  msg["type"] = type_;
  msg["sdp"] = sdp_;
  msg["receivedSessionVersion"] = -1;
  nlohmann::json config;
  config["maxVideoBW"] = max_video_bw_;
  msg["config"] = config;
  info["msg"] = msg;
  json[1] = info;

  json[2] = nlohmann::json::value_t::null;
  return json.dump();
}

ConnectionPtkBuilder& ConnectionPtkBuilder::SetErizoId(std::string id) {
  erizo_id_ = std::move(id);
  return *this;
}

ConnectionPtkBuilder& ConnectionPtkBuilder::SetConnId(std::string id) {
  connection_id_ = std::move(id);
  return *this;
}

ConnectionPtkBuilder& ConnectionPtkBuilder::SetSdpMLineIndex(int index) {
  sdp_m_line_index_ = index;
  return *this;
}

ConnectionPtkBuilder& ConnectionPtkBuilder::SetSdpMid(std::string sdpMid) {
  sdp_mid_ = std::move(sdpMid);
  return *this;
}

ConnectionPtkBuilder& ConnectionPtkBuilder::SetCandidate_(std::string candidate) {
  candidate_ = std::move(candidate);
  return *this;
}

std::string ConnectionPtkBuilder::Build() {
  auto connectionJson = nlohmann::json::array();
  connectionJson[0] = "connectionMessage";

  nlohmann::json connection;
  connection["connectionId"] = connection_id_;
  connection["erizoId"] = erizo_id_;

  nlohmann::json msg;
  msg["type"] = "candidate";
  nlohmann::json cand;
  cand["sdpMLineIndex"] = sdp_m_line_index_;
  cand["sdpMid"] = sdp_mid_;
  cand["candidate"] = candidate_;
  msg["candidate"] = cand;
  msg["receivedSessionVersion"] = received_session_version_;
  connection["msg"] = msg;

  connection["browser"] = browser_;

  connectionJson[1] = connection;
  connectionJson[2] = nlohmann::json::value_t::null;
  return connectionJson.dump();
}

CreateTokenPtkBuilder& CreateTokenPtkBuilder::SetTokenId(std::string id) {
  token_id_ = std::move(id);
  return *this;
}

CreateTokenPtkBuilder& CreateTokenPtkBuilder::SetHost(std::string host) {
  host_ = std::move(host);
  return *this;
}

CreateTokenPtkBuilder& CreateTokenPtkBuilder::SetSecure(bool secure) {
  secure_ = secure;
  return *this;
}

CreateTokenPtkBuilder& CreateTokenPtkBuilder::SetSignature(std::string signature) {
  signature_ = std::move(signature);
  return *this;
}

CreateTokenPtkBuilder& CreateTokenPtkBuilder::SetSinglePC(bool singlePC) {
  single_pc_ = singlePC;
  return *this;
}

std::string CreateTokenPtkBuilder::Build() {
  nlohmann::json tokenJson;
  tokenJson["tokenId"] = token_id_;
  tokenJson["host"] = host_;
  tokenJson["signature"] = signature_;
  tokenJson["secure"] = secure_;

  nlohmann::json data;
  data["singlePC"] = single_pc_;
  data["token"] = tokenJson;

  auto msg = nlohmann::json::array({"token", data});

  return msg.dump();
}

}