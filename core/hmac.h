//
// Created by nbaiot@126.com on 2020/10/2.
//

#ifndef LICODECPPCLIENT_HMAC_H
#define LICODECPPCLIENT_HMAC_H

#include <string>
#include <memory>

namespace nbaiot {

class HmacImpl;

class Hmac {

public:
  explicit Hmac(const std::string& key);

  ~Hmac();

  std::string Sha1(const std::string& str);

private:
  std::unique_ptr<HmacImpl> impl_;

};

}


#endif //LICODECPPCLIENT_HMAC_H
