//
// Created by nbaiot@126.com on 2020/10/16.
//

#ifndef LICODECPPCLIENT_VIDEO_RENDER_H
#define LICODECPPCLIENT_VIDEO_RENDER_H

#include "api/scoped_refptr.h"
#include "api/video/video_frame.h"
#include "api/video/video_sink_interface.h"

namespace nbaiot {

class VideoRender : public rtc::VideoSinkInterface<webrtc::VideoFrame> {

public:
  void OnFrame(const webrtc::VideoFrame& frame) override;
};


}

#endif //LICODECPPCLIENT_VIDEO_RENDER_H
