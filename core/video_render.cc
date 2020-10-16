//
// Created by nbaiot@126.com on 2020/10/16.
//

#include "video_render.h"
#include <glog/logging.h>

namespace nbaiot {

void VideoRender::OnFrame(const webrtc::VideoFrame& frame) {
  LOG(INFO) << ">>>>> receive frame:" << frame.width() << "*" << frame.height();
}

}