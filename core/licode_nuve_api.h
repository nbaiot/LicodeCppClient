//
// Created by nbaiot@126.com on 2020/9/24.
//

#ifndef LICODECPPCLIENT_LICODE_NUVE_API_H
#define LICODECPPCLIENT_LICODE_NUVE_API_H

#include <memory>
#include <vector>
#include <optional>

#include "licode_token.h"
#include "room_info.h"

namespace nbaiot {

class LicodeNuveApi {
public:
  LicodeNuveApi(std::string serviceId,
                std::string key,
                std::string url,
                int16_t port);

  std::optional<std::shared_ptr<RoomInfo>> SyncCreateRoom(const std::string& name,
                                                          const std::string& mediaConfiguration,
                                                          const std::string& roomData,
                                                          bool p2p = false);

  bool SyncDestroyRoom(const std::string& roomId);

  std::optional<std::shared_ptr<RoomInfo>> SyncGetRoom(const std::string& roomId);


  std::optional<std::vector<std::shared_ptr<RoomInfo>>> SyncListRoom();


  std::optional<LicodeToken> SyncCreateToken(const std::string& roomId,
                                             const std::string& userName,
                                             const std::string& role);

private:
  std::string CalculateSignature(const std::string& toSign, const std::string& key);

  std::string SyncRequest(const std::string& method,
                          const std::string& body,
                          const std::string& target,
                          const std::string& userName = std::string(),
                          const std::string& role = std::string()
  );


private:
  std::string service_id_;
  std::string key_;
  std::string url_;
  int16_t port_;
};

}

#endif //LICODECPPCLIENT_LICODE_NUVE_API_H
