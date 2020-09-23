//
// Created by nbaiot@126.com on 2020/9/23.
//
#include "http_url.h"

#include <regex>
#include <algorithm>

#include <glog/logging.h>

namespace nbaiot {

HttpUrl::Builder &HttpUrl::Builder::Scheme(const std::string &scheme) {
  if (scheme.empty()) {
    LOG(ERROR) << "scheme empty, use default scheme: http";
    scheme_ = "http";
  }
  if (scheme == "http")
    scheme_ = "http";
  else if (scheme == "https")
    scheme_ = "https";
  else {
    LOG(ERROR) << "unexpected scheme:" << scheme << ", use default scheme: http";
    scheme_ = "http";
  }
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::Host(const std::string &host) {
  if (host.empty()) {
    LOG(ERROR) << "host empty";
    return *this;
  } else {
    host_.clear();
    std::transform(host.begin(), host.end(), host_.begin(), ::tolower);
  }
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::Port(int port) {
  if (port <= 0 || port > 65535) {
    LOG(ERROR) << "unexpected port:" << port;
    port_ = -1;
  } else {
    port_ = port;
  }
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::AddEncodedPathSegment(const std::string &pathSegment) {
  if (pathSegment.empty()) {
    LOG(ERROR) << "pathSegments empty";
    return *this;
  }
  Push(pathSegment);
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::AddEncodedPathSegments(const std::string &pathSegment) {
  if (pathSegment.empty()) {
    LOG(ERROR) << "pathSegments empty";
    return *this;
  }
  std::regex re{"/"};
  auto ss = std::vector<std::string>{std::sregex_token_iterator(pathSegment.begin(), pathSegment.end(), re, -1),
                                     std::sregex_token_iterator()};
  for (auto &it : ss) {
    if (!it.empty())
      Push(it);
  }
  return *this;
}

/**
 * [""] ==> [""]
 * ["a", "b", "c"] ==> ["a", "b", ""]
 * ["a", "b", "c", ""] ==> ["a", "b", ""]
 */
void HttpUrl::Builder::Pop() {
  if (!encoded_path_segments_.empty()) {
    auto last = --encoded_path_segments_.end();
    encoded_path_segments_.erase(last);
  }
}

void HttpUrl::Builder::Push(const std::string &segment) {
  if (segment.empty())
    return;
  /// TODO: encode segment
  if (segment == ".") {
    return;
  }
  if (segment == "..") {
    Pop();
    return;
  }
  encoded_path_segments_.emplace_back(segment);
}

HttpUrl::Builder &HttpUrl::Builder::SetEncodedPathSegment(int index, const std::string &pathSegment) {
  if (pathSegment.empty()) {
    LOG(ERROR) << "pathSegments empty";
    return *this;
  }
  if (pathSegment == "." || pathSegment == "..") {
    LOG(ERROR) << "unexpected pathSegment:" << pathSegment;
    return *this;
  }
  if (index < 0 || index >= encoded_path_segments_.size()) {
    LOG(ERROR) << "unexpected index:" << index;
    return *this;
  }
  auto it = encoded_path_segments_.begin();
  for (int i = 0; i < index; ++i) {
    ++it;
  }
  *it = pathSegment;
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::RemovePathSegment(int index) {
  if (index < 0 || index >= encoded_path_segments_.size()) {
    LOG(ERROR) << "unexpected index:" << index;
    return *this;
  }
  auto it = encoded_path_segments_.begin();
  for (int i = 0; i < index; ++i) {
    ++it;
  }
  encoded_path_segments_.erase(it);
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::AddEncodedQueryParameter(const std::string& name, const std::string& value) {
  if (name.empty()) {
    LOG(ERROR) << "unexpected name:" << name;
    return *this;
  }
  auto it = encoded_query_names_and_values_.find(name);
  if (it == encoded_query_names_and_values_.end()) {
    std::set<std::string> values;
    values.emplace(value);
    encoded_query_names_and_values_.emplace(std::make_pair(name, values));
  } else {
    it->second.emplace(value);
  }
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::RemoveEncodedQueryParameter(const std::string &name, const std::string &value) {
  if (name.empty()) {
    LOG(ERROR) << "unexpected name:" << name;
    return *this;
  }
  auto it = encoded_query_names_and_values_.find(name);
  if (it != encoded_query_names_and_values_.end()) {
    it->second.erase(value);
    if (it->second.empty()) {
      encoded_query_names_and_values_.erase(name);
    }
  }
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::RemoveEncodedQueryParameter(const std::string &name) {
  if (name.empty()) {
    LOG(ERROR) << "unexpected name:" << name;
    return *this;
  }
  encoded_query_names_and_values_.erase(name);
  return *this;
}

HttpUrl::Builder &HttpUrl::Builder::AddEncodedFragment(const std::string &fragment) {
  if (fragment.empty()) {
    LOG(ERROR) << "unexpected fragment:" << fragment;
    return *this;
  }
  encoded_fragment_ = fragment;
  return *this;
}

HttpUrl HttpUrl::Builder::Build() {
  if (scheme_.empty() || host_.empty()) {
    LOG(ERROR) << "scheme or host empty";
  }
  return HttpUrl(*this);
}

bool HttpUrl::Builder::Parse(std::string url) {
  if (url.empty() || url.size() <= 7) {
    return false;
  }

  Clear();

  url.erase(0, url.find_first_not_of(' '));
  url.erase(0, url.find_first_not_of('\t'));
  url.erase(0, url.find_first_not_of('\n'));
  url.erase(0, url.find_first_not_of('\f'));
  url.erase(0, url.find_first_not_of('\r'));

  url.erase(url.find_last_not_of(' ') +  1);
  url.erase(url.find_last_not_of('\t') + 1);
  url.erase(url.find_last_not_of('\n') + 1);
  url.erase(url.find_last_not_of('\f') + 1);
  url.erase(url.find_last_not_of('\r') + 1);

  /// scheme
  auto scheme = url.substr(0, url.find_first_of(':'));
  if (scheme.empty()) {
    LOG(ERROR) << "unexpected url:" << url;
    return false;
  }
  std::transform(scheme.begin(), scheme.end(), scheme.begin(), ::tolower);
  if (scheme == "http") {
    scheme_ = "http";
  } else if (scheme == "https") {
    scheme_ = "https";
  } else {
    LOG(ERROR) << "unexpected url:" << url;
    return false;
  }

  /// host+path
  /// TODO: fixme, ignore username and password
  auto pathStart = scheme_.size() + 3;/// ://
  auto pathEnd = url.find('?');
  if (pathEnd == std::string::npos) {
    pathEnd = url.find('#');
  }
  if (pathEnd == std::string::npos) {
    pathEnd = url.size();
  }
  auto hostPath =  url.substr(pathStart, pathEnd - pathStart);

  if (hostPath.empty()) {
    LOG(ERROR) << "unexpected url:" << url;
    return false;
  }

  /// host
  auto portStart = hostPath.find_first_of(':');
  pathStart = hostPath.find_first_of('/');

  if (portStart == std::string::npos) {
    /// port
    port_ = DefaultPort(scheme_);
    if (pathStart ==  std::string::npos) {
      host_ = hostPath;
    } else {
      host_ = hostPath.substr(0, pathStart);
    }
  } else {
    host_ = hostPath.substr(0, portStart);
    auto portEnd = hostPath.find_first_of('/');
    std::string portS;
    if (portEnd == std::string::npos) {
      portS = hostPath.substr(portStart + 1);
    } else {
      portS = hostPath.substr(portStart + 1, portEnd - (portStart + 1));
    }
    if (!portS.empty()) {
      /// TODO: fixme, not number?
      port_ = std::atoi(portS.c_str());
    } else {
      port_ = DefaultPort(scheme_);
    }
  }

  /// path
  auto path = hostPath.substr(pathStart + 1);
  if (!path.empty()) {
    /// TODO: fixme, encode path
    AddEncodedPathSegments(path);
  }

  auto queryStart = url.find_first_of('?');
  auto fragmentStart = url.find_first_of('#');
  /// query
  std::string query;
  if (queryStart != std::string::npos) {
    if (fragmentStart == std::string::npos) {
      query = url.substr(queryStart + 1);
    } else {
      query = url.substr(queryStart + 1, fragmentStart - (queryStart + 1));
    }
  }
  /// TODO: fixme, encode query
  if (!query.empty()) {
    std::regex re{"&"};
    auto ss = std::vector<std::string>{std::sregex_token_iterator(query.begin(), query.end(), re, -1),
                                       std::sregex_token_iterator()};
    for (auto &s : ss) {
      if (!s.empty()) {
        std::string param;
        std::string value;
        /// find '='
        auto p = s.find_first_of('=');
        if (p != std::string::npos) {
          param = s.substr(0, p);
          value = s.substr(p + 1);
        } else {
          param = s;
        }
        AddEncodedQueryParameter(param, value);
      }
    }
  }

  /// fragment
  /// TODO: fixme, encode fragment
  if (fragmentStart != std::string::npos) {
    AddEncodedFragment(url.substr(fragmentStart + 1));
  }

  return true;
}

std::string HttpUrl::Builder::ToString() const {
  std::ostringstream url;
  url << scheme_;
  url << "://";
  /// TODO: fixme, add username and password, eg: username:password@

  /// ipv6
  if (host_.find(':') != std::string::npos) {
    url << '[';
    url << host_;
    url << ']';
  } else {
    url << host_;
  }

  int port = (port_ == -1) ? DefaultPort(scheme_) : port_;
  url << ':';
  url << port;

  for (auto& i: encoded_path_segments_) {
    url << '/';
    url << i;
  }

  if (!encoded_query_names_and_values_.empty()) {
    url << '?';
    int paramIndex = 0;
    for(auto& i : encoded_query_names_and_values_) {
      auto param = i.first;
      auto values = i.second;;
      if (values.empty()) {
        url << param;
      } else {
        int valuesIndex = 0;
        for(auto & value : values) {
          url << param;
          url << '=';
          url << value;
          ++valuesIndex;
          if (valuesIndex < values.size()) {
            url << '&';
          }
        }
        ++paramIndex;
        if (paramIndex < encoded_query_names_and_values_.size()) {
          url << '&';
        }
      }
    }
  }

  if (!encoded_fragment_.empty()) {
    url << '#';
    url << encoded_fragment_;
  }

  return url.str();
}


int HttpUrl::DefaultPort(const std::string &scheme) {
  if (scheme == "http")
    return 80;
  else if (scheme == "https")
    return 443;
  return -1;
}

HttpUrl::HttpUrl(std::string url) : port_(-1) {
  Builder builder;
  if (builder.Parse(std::move(url))) {
    Init(builder);
  }
}

HttpUrl::HttpUrl(const HttpUrl &url) {
  scheme_ = url.scheme_;
  host_ = url.host_;
  port_ = url.port_;
  path_segments_ = url.path_segments_;
  query_names_and_values_ = url.query_names_and_values_;
  fragment_ = url.fragment_;
  url_ = url.url_;
}

HttpUrl::HttpUrl(const Builder& builder) : port_(-1) {
  Init(builder);
}

void HttpUrl::Init(const Builder &builder) {
  scheme_ = builder.scheme_;
  host_ = builder.host_;
  port_ = builder.port_;
  /// TODO: decoded
  path_segments_ = builder.encoded_path_segments_;
  query_names_and_values_ = builder.encoded_query_names_and_values_;
  fragment_ = builder.encoded_fragment_;
  url_ = builder.ToString();
}

std::string HttpUrl::Scheme() {
  return scheme_;
}

int HttpUrl::Port() {
  return port_;
}

bool HttpUrl::IsHttps() {
  return scheme_ == "https";
}

std::string HttpUrl::Host() {
  return host_;
}

int HttpUrl::PathSize() {
  /// 始终包含 ""
  return (int)path_segments_.size() - 1;
}

std::string HttpUrl::EncodedPath() {
  auto pathStart = url_.find_first_of('/', scheme_.size() + 3);/// ://
  auto pathEnd = url_.find('?');
  if (pathEnd == std::string::npos) {
    pathEnd = url_.find('#');
  }
  if (pathEnd == std::string::npos) {
    pathEnd = url_.size();
  }
  return url_.substr(pathStart, pathEnd - pathStart);
}

std::string HttpUrl::EncodedQuery() {
  auto queryStart = url_.find('?');
  auto queryEnd = url_.find('#');
  if (queryStart != std::string::npos) {
    if (queryEnd != std::string::npos) {
      /// ignore '?', '#'
      return url_.substr(queryStart + 1, queryEnd - (queryStart + 1));
    } else {
      /// ignore '?'
      return url_.substr(queryStart + 1);
    }
  }
  return "";
}

int HttpUrl::QuerySize() {
  return query_names_and_values_.size();
}

std::set<std::string> HttpUrl::QueryParameter() {
  return std::set<std::string>();
}

std::string HttpUrl::EncodedFragment() {
  return fragment_;
}

std::string HttpUrl::Url() {
  return url_;
}

}