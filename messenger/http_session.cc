//
// Created by nbaiot@126.com on 2020/9/23.
//

#include "http_session.h"

#include <boost/asio/strand.hpp>

namespace nbaiot {

HttpSession::HttpSession(boost::asio::io_context& ioc)
    : resolver_(boost::asio::make_strand(ioc)), stream_(boost::asio::make_strand(ioc)) {

}

HttpSession::~HttpSession() {

}

std::shared_ptr<HttpResponse<StringHttpBody, StringHttpBody>>
HttpSession::Post(const std::shared_ptr<HttpRequest<StringHttpBody>>& request) {
  auto host = request->Url().Host();
  auto port = request->Url().Port();
  auto response = std::make_shared<HttpResponse<StringHttpBody, StringHttpBody>>();
  response->SetRequest(request->Request());
  try {
    resolver_.async_resolve(host, std::to_string(port), [response, self = shared_from_this()](boost::beast::error_code ec,
                                                           const boost::asio::ip::tcp::resolver::results_type& results) {
        if (ec) {
          LOG(ERROR) << ">>>>>HttpSession resolve error:" << ec.message();
          return;
        }
        /// Set the timeout for the operation
        self->stream_.expires_after(std::chrono::seconds(30));
        /// Make the connection on the IP address we get from a lookup
        self->stream_.async_connect(
            results,[response, self](boost::beast::error_code ec,
                       boost::asio::ip::tcp::resolver::results_type::endpoint_type ep) {
                if (ec) {
                  LOG(ERROR) << ">>>>>HttpSession connect error:" << ec.message();
                  return;
                }
                self->stream_.expires_after(std::chrono::seconds(30));
                LOG(ERROR) << ">>>>>HttpSession connect success:" << ec.message();
                auto tt = response->Request();
                LOG(ERROR) << ">>>>>HttpSession resolve success:" << tt.method_string();
                LOG(ERROR) << ">>>>>HttpSession resolve success:" << tt.target();
                boost::beast::http::async_write(self->stream_, tt,
                    [self](boost::beast::error_code ec, std::size_t bytes_transferred) {
                        if (ec) {
                          LOG(ERROR) << ">>>>>HttpSession write error:" << ec.message();
                          return;
                        }
                        LOG(ERROR) << ">>>>>HttpSession write success:" << ec.message();
                });
            });
    });
  } catch (const boost::system::system_error& error) {
    LOG(ERROR) << ">>>>>>>>> HttpSession resolve " << host << ":" << port
               << " failed, error:" << error.what();
  }

  return response;
}

}