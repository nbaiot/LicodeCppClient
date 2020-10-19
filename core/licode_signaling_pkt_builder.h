//
// Created by nbaiot@126.com on 2020/9/25.
//

#ifndef LICODECPPCLIENT_LICODE_SIGNALING_PKT_BUILDER_H
#define LICODECPPCLIENT_LICODE_SIGNALING_PKT_BUILDER_H

#include <string>
#include "licode_token.h"

namespace nbaiot {

class PublicStreamPktBuilder {
public:
  PublicStreamPktBuilder& SetLabel(std::string label);

  PublicStreamPktBuilder& SetState(std::string state);

  std::string Build();
  /// TODO: add other params

  /// TODO: add attributes

private:
  std::string state_;
  std::string label_;
  bool video_{true};
  bool audio_{true};
  bool data_{true};
  bool screen_{false};
  bool mute_video_{false};
  bool mute_audio_{false};
  int min_video_bw_{0};
  int max_video_bw_{300};
};

class updateStreamAttributesPktBuilder {
public:
  updateStreamAttributesPktBuilder& SetStreamId(uint64_t streamId);

  updateStreamAttributesPktBuilder& SetTypeAttr(std::string type);

  std::string Build();

private:
  uint64_t stream_id_;
  std::string type_attr_;
};


class SubscribeStreamPktBuilder {

public:
  SubscribeStreamPktBuilder& SetStreamId(uint64_t streamId);

  SubscribeStreamPktBuilder& SetOfferFromErizo(bool enable);

  std::string Build();

private:
  uint64_t stream_id_;
  bool video_{true};
  bool audio_{true};
  bool data_{true};
  bool mute_video_{false};
  bool mute_audio_{false};
  bool offer_from_erizo_{true};
  int max_video_bw_{300};
  std::string browser_{"cpp"};
};

class UnsubscribeStreamPktBuilder {
public:
  UnsubscribeStreamPktBuilder& SetStreamId(uint64_t streamId);

  std::string Build();

private:
  uint64_t stream_id_;
};

class OfferOrAnswerPktBuilder {
public:
  OfferOrAnswerPktBuilder& SetErizoId(std::string id);

  OfferOrAnswerPktBuilder& SetConnId(std::string id);

  OfferOrAnswerPktBuilder& SetType(std::string type);

  OfferOrAnswerPktBuilder& SetSdp(std::string sdp);

  std::string Build();

private:
  std::string browser_{"cpp"};
  std::string erizo_id_;
  std::string connection_id_;

  std::string type_;
  std::string sdp_;
  int received_session_version_{-1};
  int max_video_bw_{300};
};

class ConnectionPtkBuilder {
public:
  ConnectionPtkBuilder& SetErizoId(std::string id);

  ConnectionPtkBuilder& SetConnId(std::string id);

  ConnectionPtkBuilder& SetSdpMLineIndex(int index);

  ConnectionPtkBuilder& SetSdpMid(std::string sdpMid);

  ConnectionPtkBuilder& SetCandidate_(std::string candidate);

  std::string Build();

private:
  std::string browser_{"cpp"};
  std::string erizo_id_;
  std::string connection_id_;

  int sdp_m_line_index_;
  std::string sdp_mid_;
  std::string candidate_;
  int received_session_version_{0};
};

class CreateTokenPtkBuilder {
public:
  CreateTokenPtkBuilder& SetTokenId(std::string id);

  CreateTokenPtkBuilder& SetHost(std::string host);

  CreateTokenPtkBuilder& SetSecure(bool secure);

  CreateTokenPtkBuilder& SetSignature(std::string signature);

  CreateTokenPtkBuilder& SetSinglePC(bool singlePC);

  std::string Build();

private:
  std::string token_id_;
  std::string host_;
  bool secure_;
  std::string signature_;
  bool single_pc_{false};
};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_PKT_BUILDER_H
