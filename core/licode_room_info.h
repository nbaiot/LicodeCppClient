//
// Created by nbaiot@126.com on 2020/10/3.
//

#ifndef LICODECPPCLIENT_LICODE_ROOM_INFO_H
#define LICODECPPCLIENT_LICODE_ROOM_INFO_H

#include <string>

namespace nbaiot {

class LicodeRoomInfo {

public:
  LicodeRoomInfo(std::string id, std::string name, std::string data, std::string mediaConfiguration, bool p2p)
      : id_(std::move(id)), name_(std::move(name)), data_(std::move(data)),
        media_configuration_(std::move(mediaConfiguration)), p2p_(p2p) {}

  std::string Id() {
    return id_;
  }

  std::string Name() {
    return name_;
  }

  std::string Data() {
    return data_;
  }

  std::string MediaConfiguration() {
    return media_configuration_;
  }

  bool P2p() {
    return p2p_;
  }

private:
  std::string id_;
  std::string name_;
  /// TODO: fixme
  std::string data_;
  std::string media_configuration_;
  bool p2p_;
};

}

#endif //LICODECPPCLIENT_LICODE_ROOM_INFO_H
