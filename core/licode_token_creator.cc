//
// Created by cuangenglei-os@360os.com on 2020/9/24.
//

#include "licode_token_creator.h"

#include <memory>
#include <thread>
#include <future>

#include <boost/asio/io_context.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <nlohmann/json.hpp>

#include "messenger/http_defines.h"
#include "messenger/http_request.h"

#include <glog/logging.h>

namespace nbaiot {

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
      SyncPost(std::shared_ptr<HttpRequest<StringHttpBody>> request, int timeoutMS) {
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

LicodeToken LicodeTokenCreator::SyncCreate(const std::string& url, int timeoutMS) {
  auto http = std::make_shared<SimpleHttpSession>();
  auto request = std::make_shared<HttpRequest<StringHttpBody>>();
  request->SetUrl(url);
  request->SetMethod("POST");
  request->SetVersion(kHTTP_1_1);
  request->AddHeader("User-Agent", BOOST_BEAST_VERSION_STRING);
  request->AddHeader("Content-Type", "application/json");
  nlohmann::json body;
  body["mediaConfiguration"] = false;
  body["room"] = false;
  body["type"] = false;
  body["username"] = "jack";
  body["role"] = "presenter";
  request->Body() = body.dump();
  request->PreparePayload();
  LicodeToken token;
  auto response = http->SyncPost(request, timeoutMS);
  if (response.result() == boost::beast::http::status::ok) {
    auto base64 = response.body();
    using namespace boost::archive::iterators;
    typedef transform_width<binary_from_base64<std::string::const_iterator>, 8, 6> binary_t;
    std::string dec(binary_t(base64.begin()), binary_t(base64.end()));
    auto result = nlohmann::json::parse(dec);
    token.signature = result.value<std::string>("signature", "");
    token.host = result.value<std::string>("host", "");
    token.tokenId = result.value<std::string>("tokenId", "");
    token.secure = result.value<bool>("secure", false);
    LOG(INFO) << ">>>>>>>:\n" << result.dump(4);
  } else {
    LOG(ERROR) << "create token error:" << response.reason();
  }
  return token;
}
}