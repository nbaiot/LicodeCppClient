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

  static std::string CreateSubscribeStreamPkt(uint64_t streamId);

  /// TODO: fixme config
  static std::string CreateConnectionOfferMsg(const std::string& sdp, int maxVideoBW);

  static std::string CreateConnectionAnswerMsg(const std::string& sdp, int maxVideoBW);

  static std::string CreateConnectionCandidateMsg(int sdpMLineIndex,
                                                  const std::string& sdpMid,
                                                  const std::string& candidate);

  static std::string CreateConnectionPtk(const std::string& connectionId,
                                         const std::string& erizoId,
                                         const std::string& msg);

  static std::string CreateOfferOrAnswerPkt(bool offer,
                                            const std::string& erizoId,
                                            const std::string& connId,
                                            const std::string& sdp,
                                            int maxVideoBW);

};

}

#endif //LICODECPPCLIENT_LICODE_SIGNALING_PKT_CREATOR_H
