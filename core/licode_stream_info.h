//
// Created by nbaiot@126.com on 2020/10/2.
//

#ifndef LICODECPPCLIENT_LICODE_STREAM_INFO_H
#define LICODECPPCLIENT_LICODE_STREAM_INFO_H

#include <string>

namespace nbaiot {

class LicodeStreamInfo {

public:
  LicodeStreamInfo(uint64_t id,
                   bool hasVideo,
                   bool hasAudio,
                   bool hasData,
                   std::string label,
                   bool screen,
                   std::string attributes)
      : id_(id), has_video_(hasVideo),
        has_audio_(hasAudio), has_data_(hasData),
        label_(std::move(label)),
        screen_(screen), attributes_(std::move(attributes)) {

  }

  LicodeStreamInfo() : LicodeStreamInfo(0, false, false, false, "", false, "") {}

  uint64_t Id() {
    return id_;
  }

  std::string Label() {
    return label_;
  }

  bool HasVideo() {
    return has_video_;
  }

  bool HasAudio() {
    return has_audio_;
  }

  bool HasData() {
    return has_data_;
  }

  bool screen() {
    return screen_;
  }

  std::string Attributes() {
    return attributes_;
  }


public:
  uint64_t id_;
  bool has_video_;
  bool has_audio_;
  bool has_data_;
  std::string label_;
  bool screen_;
  /// type: public private
  std::string attributes_;

};

}

#endif //LICODECPPCLIENT_LICODE_STREAM_INFO_H
