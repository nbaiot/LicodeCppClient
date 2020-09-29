//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_NUVE_API_H
#define LICODECPPCLIENT_LICODE_NUVE_API_H

#include "licode_token.h"

namespace nbaiot {

class LicodeNuveApi {
public:
  enum Roles {
    kPresenter,
    kViewer,
    kViewerWithData,
  };

  enum Type {
    kP2p,
    kErizo,
  };

  /// room 相关:
  /// licode/nuve/nuveAPI/resource/roomsResource.js
  /// licode/nuve/nuveAPI/resource/roomResource.js
  ///
  static LicodeToken SyncCreateToken(const std::string& url,
                                     const std::string& userName,
                                     const std::string& RoomName,
                                     Roles role,
                                     Type type,
                                     int timeoutMS = 3000);
};

}

#endif //LICODECPPCLIENT_LICODE_NUVE_API_H
