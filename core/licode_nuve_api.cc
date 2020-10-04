//
// Created by nbaiot@126.com on 2020/9/24.
//

#include "licode_nuve_api.h"

#include <memory>
#include <thread>
#include <future>
#include <random>

#include <boost/asio/io_context.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>

#include <nlohmann/json.hpp>

#include "messenger/http_defines.h"
#include "messenger/http_request.h"
#include "hmac.h"
#include <glog/logging.h>

namespace nbaiot {

static const std::string baseHeader = "MAuth realm=http://marte3.dit.upm.es,mauth_signature_method=HMAC_SHA1";

class SimpleHttpSession : public std::enable_shared_from_this<SimpleHttpSession> {

public:
  SimpleHttpSession() {
    thread_ = std::make_unique<std::thread>([this]() {
      ioc_.run();
    });
    promise_ = std::make_shared<std::promise<bool>>();
  }

  ~SimpleHttpSession() {
    ioc_.stop();
    if (thread_ && thread_->joinable()) {
      thread_->join();
    }
  }

  boost::beast::http::response<StringHttpBody>
  SyncRequest(std::shared_ptr<HttpRequest<StringHttpBody>> request, int timeoutMS) {
    response_.clear();
    request_ = std::move(request);
    timeoutMS_ = timeoutMS;
    auto host = request_->Url().Host();
    auto port = request_->Url().Port();
    resolver_.async_resolve(host, std::to_string(port),
                            boost::beast::bind_front_handler(&SimpleHttpSession::OnResolve, shared_from_this()));

    auto success = promise_->get_future().get();
    return response_;
  }

private:
  void OnResolve(boost::beast::error_code ec,
                 const boost::asio::ip::tcp::resolver::results_type& results) {
    if (ec) {
      LOG(ERROR) << ">>>>>>>>>> resolve " << request_->Url().Host() << ":" << request_->Url().Port()
                 << " error:" << ec.message();
      promise_->set_value(false);
      response_.result(boost::beast::http::status::unknown);
      response_.reason("resolve failed");
      return;
    }

    stream_.expires_after(std::chrono::milliseconds(timeoutMS_));
    stream_.async_connect(results,
                          boost::beast::bind_front_handler(&SimpleHttpSession::OnConnect, shared_from_this()));
  }

  void OnConnect(boost::beast::error_code ec,
                 const boost::asio::ip::tcp::resolver::results_type::endpoint_type& ep) {
    if (ec) {
      LOG(ERROR) << ">>>>>>>>>> connect " << request_->Url().Host() << ":" << request_->Url().Port()
                 << " error:" << ec.message();
      promise_->set_value(false);
      response_.result(boost::beast::http::status::unknown);
      response_.reason("connect failed");
      return;
    }
    boost::beast::http::async_write(stream_, request_->Request(),
                                    boost::beast::bind_front_handler(&SimpleHttpSession::OnWrite, shared_from_this()));

  }

  void OnWrite(boost::beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
      LOG(ERROR) << ">>>>>>>>>> write message to " << request_->Url().Host() << ":" << request_->Url().Port()
                 << " error:" << ec.message();
      promise_->set_value(false);
      response_.result(boost::beast::http::status::unknown);
      response_.reason("write failed");
      return;
    }
    boost::ignore_unused(bytes_transferred);
    boost::beast::http::async_read(stream_, buffer_, response_,
                                   boost::beast::bind_front_handler(&SimpleHttpSession::OnRead, shared_from_this()));

  }

  void OnRead(boost::beast::error_code ec,
              std::size_t bytes_transferred) {
    if (ec) {
      LOG(ERROR) << ">>>>>>>>>> read message from " << request_->Url().Host() << ":" << request_->Url().Port()
                 << " error:" << ec.message();
      promise_->set_value(false);
      response_.result(boost::beast::http::status::unknown);
      response_.reason("read failed");
      return;
    }

    stream_.socket().shutdown(boost::asio::ip::tcp::socket::shutdown_both, ec);
    if (ec && ec != boost::beast::errc::not_connected) {
      LOG(ERROR) << ">>>>>>> shutdown " << request_->Url().Host() << ":" << request_->Url().Port()
                 << " error:" << ec.message();
    }
    promise_->set_value(true);
  }

private:
  std::shared_ptr<std::promise<bool>> promise_;
  int timeoutMS_{3000};
  boost::asio::io_context ioc_;
  boost::asio::executor_work_guard<boost::asio::io_context::executor_type> work_guard{ioc_.get_executor()};
  boost::asio::ip::tcp::resolver resolver_{ioc_};
  boost::beast::tcp_stream stream_{ioc_};
  boost::beast::flat_buffer buffer_;
  std::shared_ptr<HttpRequest<StringHttpBody>> request_;
  boost::beast::http::response<StringHttpBody> response_;
  std::unique_ptr<std::thread> thread_;
};

LicodeNuveApi::LicodeNuveApi(std::string serviceId, std::string key, std::string url, int16_t port)
    : service_id_(std::move(serviceId)), key_(std::move(key)), url_(std::move(url)), port_(port) {

}

std::optional<std::shared_ptr<RoomInfo>> LicodeNuveApi::SyncCreateRoom(const std::string& name,
                                                                       const std::string& mediaConfiguration,
                                                                       const std::string& roomData, bool p2p) {

  std::string target = "/rooms";

  nlohmann::json body;
  body["name"] = name;
  nlohmann::json options;
  options["mediaConfiguration"] = "default";
  options["data"] = roomData;
  if (p2p) {
    options["p2p"] = true;
  }
  body["options"] = options;
  auto result = SyncRequest("POST", body.dump(), target);
  if (result.empty())
    return std::nullopt;

  try {
    auto response = nlohmann::json::parse(result);

    return std::optional(std::make_shared<RoomInfo>(response.value<std::string>("_id", ""),
                                                    response.value<std::string>("name", ""),
                                                    response.value<std::string>("data", ""),
                                                    response.value<std::string>("mediaConfiguration", ""),
                                                    response.value<bool>("p2p", false)));
  } catch (const std::exception& e) {
    LOG(ERROR) << ">>>>>>>>>>SyncCreateRoom error:" << e.what();
    return std::nullopt;
  }

}

bool LicodeNuveApi::SyncDestroyRoom(const std::string& roomId) {
  std::string target = "/rooms/" + roomId;
  auto result = SyncRequest("DELETE", "", target);
  return result == "Room deleted";
}

std::optional<std::shared_ptr<RoomInfo>> LicodeNuveApi::SyncGetRoom(const std::string& roomId) {
  std::string target = "/rooms/" + roomId;
  auto result = SyncRequest("GET", "", target);
  if (result.empty())
    return std::nullopt;
  try {
    auto response = nlohmann::json::parse(result);

    return std::optional(std::make_shared<RoomInfo>(response.value<std::string>("_id", ""),
                                                    response.value<std::string>("name", ""),
                                                    response.value<std::string>("data", ""),
                                                    response.value<std::string>("mediaConfiguration", ""),
                                                    response.value<bool>("p2p", false)));
  } catch (const std::exception& e) {
    LOG(ERROR) << ">>>>>>>>>>SyncGetRoom error:" << e.what();
    return std::nullopt;
  }
}

std::optional<LicodeToken>
LicodeNuveApi::SyncCreateToken(const std::string& roomId, const std::string& userName, const std::string& role) {
  std::string target = "/rooms/" + roomId + "/tokens";
  auto result = SyncRequest("POST", "", target, userName, role);
  if (result.empty())
    return std::nullopt;

  using namespace boost::archive::iterators;
  typedef transform_width<binary_from_base64<std::string::const_iterator>, 8, 6> binary_t;
  std::string dec(binary_t(result.begin()), binary_t(result.end()));
  try {
    auto token = nlohmann::json::parse(dec);
    LicodeToken licodeToken;
    licodeToken.signature = token.value<std::string>("signature", "");
    licodeToken.host = token.value<std::string>("host", "");
    licodeToken.tokenId = token.value<std::string>("tokenId", "");
    licodeToken.secure = token.value<bool>("secure", false);
    return std::optional(licodeToken);
  } catch (const std::exception& e) {
    LOG(ERROR) << ">>>>>:" << e.what();
  }
  return std::optional<LicodeToken>();
}


std::optional<std::vector<std::shared_ptr<RoomInfo>>> LicodeNuveApi::SyncListRoom() {

  std::string target = "/rooms";
  auto result = SyncRequest("GET", "", target);
  if (result.empty())
    return std::nullopt;

  try {
    auto response = nlohmann::json::parse(result);
    std::vector<std::shared_ptr<RoomInfo>> rooms;
    rooms.reserve(response.size());
    for (auto& room : response) {
      std::string name;
      if (room["name"].is_string()) {
        name = room.value<std::string>("name", "");
      }
      std::string data;
      /// TODO:fixme
      if (!room["data"].is_null()) {
        if (room["data"].is_object()) {
          data = room["data"].dump();
        } else {
          data = room["data"];
        }
      }
      std::string mediaConfiguration;
      if (!room["mediaConfiguration"].is_null()) {
        mediaConfiguration = room["mediaConfiguration"];
      }
      rooms.push_back(std::make_shared<RoomInfo>(room.value<std::string>("_id", ""),
                                                 room.value<std::string>("name", ""),
                                                 data,
                                                 mediaConfiguration,
                                                 false));
    }
    return std::optional(rooms);
  } catch (const std::exception& e) {
    LOG(ERROR) << ">>>>>>>>>>SyncCreateRoom error:" << e.what();
    return std::nullopt;
  }

}

std::string LicodeNuveApi::CalculateSignature(const std::string& toSign, const std::string& key) {
  Hmac hmac(key);
  auto hash = hmac.Sha1(toSign);
  using namespace boost::archive::iterators;
  std::stringstream result;
  typedef base64_from_binary<transform_width<std::string::const_iterator, 6, 8>> binary_t;
  std::copy(binary_t(hash.begin()), binary_t(hash.end()), std::ostream_iterator<char>(result));
  size_t equal_count = (3 - hash.length() % 3) % 3;
  for (size_t i = 0; i < equal_count; i++) {
    result.put('=');
  }
  return result.str();
}

std::string LicodeNuveApi::SyncRequest(const std::string& method,
                                       const std::string& body,
                                       const std::string& target,
                                       const std::string& userName,
                                       const std::string& role
) {
  auto http = std::make_shared<SimpleHttpSession>();
  auto request = std::make_shared<HttpRequest<StringHttpBody>>();
  auto url = "http://" + url_ + ":" + std::to_string(port_) + target;
  request->SetUrl(url);
  request->SetMethod(method);
  request->SetVersion(kHTTP_1_1);
  request->AddHeader("User-Agent", BOOST_BEAST_VERSION_STRING);

  auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
      std::chrono::system_clock::now().time_since_epoch()).count();
  std::random_device rd;
  auto cnounce = std::default_random_engine(rd())();
  auto toSign = std::to_string(timestamp) + "," + std::to_string(cnounce);

  std::ostringstream header;
  header << baseHeader;

  if (!userName.empty() && !role.empty()) {
    header << ",,mauth_username=" << userName;
    header << ",,,mauth_role=" << role;
    toSign += "," + userName + "," + role;
  }

  header << ",mauth_serviceid=" << service_id_;
  header << ",mauth_cnonce=" << cnounce;
  header << ",mauth_timestamp=" << timestamp;;
  header << ",mauth_signature=" << CalculateSignature(toSign, key_);
  request->AddHeader("Authorization", header.str());
  request->AddHeader("Content-Type", "application/json");

  /// TODO: fixme
  if (method != "GET" && !body.empty()) {
    request->Body() = body;
  }
  request->PreparePayload();
  std::string result;
  auto response = http->SyncRequest(request, 3000);
  if (response.result() == boost::beast::http::status::ok) {
    result = response.body();
    LOG(INFO) << ">>>>>>>>>>>>>> result:" << result;
  } else {
    LOG(ERROR) << "SyncRequest error:" << response.reason();
  }
  return result;
}

}