//
// Created by nbaiot@126.com on 2020/10/15.
//

#ifndef LICODECPPCLIENT_WEBRTC_WRAPPER_H
#define LICODECPPCLIENT_WEBRTC_WRAPPER_H

#include <memory>

#include "api/scoped_refptr.h"
#include "api/jsep.h"

namespace rtc {
class Thread;
}

namespace webrtc {
class PeerConnectionFactoryInterface;
}

namespace nbaiot {

class WebrtcWrapper {

public:
  static WebrtcWrapper* Instance();

  void Init();

  rtc::Thread* SignalingThread();

  rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> CreatePCFactory();

  std::unique_ptr<webrtc::SessionDescriptionInterface> CreateSessionDescription(webrtc::SdpType type,
                                                                                const std::string& sdp,
                                                                                webrtc::SdpParseError* error_out);

private:
  WebrtcWrapper() = default;

  ~WebrtcWrapper() = default;

private:
  bool init_{false};
  std::unique_ptr<rtc::Thread> signaling_thread_;

};

}

#endif //LICODECPPCLIENT_WEBRTC_WRAPPER_H
