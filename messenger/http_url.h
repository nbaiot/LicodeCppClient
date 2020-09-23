//
// Created by nbaiot@126.com on 2020/9/23.
//
#pragma once

#ifndef LICODECPPCLIENT_HTTP_URL_H
#define LICODECPPCLIENT_HTTP_URL_H

#include <map>
#include <set>
#include <list>
#include <string>
#include <vector>

namespace nbaiot {
/**
    http://example.com:8888/over/there?name=ferret#nose
    \__/ \______________/ \________/\_________/ \__/

    |         |              |         |        |

 scheme     host           path      query   fragment
 */

/**
 * https://www.cnblogs.com/liuhongfeng/p/5006341.html
 */
class HttpUrl {

public:
  class Builder {
  public:
    Builder() = default;

    Builder& Scheme(const std::string& scheme);

    /**
     *
     * @param host
     * either a regular hostname, International Domain Name, IPv4 address
     * TODO: add IPv6 address
     * @return
     */
    Builder& Host(const std::string& host);

    Builder& Port(int port);

    /**
     * 单一 segment
     * @param pathSegment
     * @return
     */
    Builder& AddEncodedPathSegment(const std::string& pathSegment);

    /// TODO: fixme
    ///Builder& AddPathSegment(const std::string& pathSegment);

    /**
     * 组合 segment
     * eg:/a/b/c/ or a/b/c
     * @param pathSegment
     * @return
     */
    Builder& AddEncodedPathSegments(const std::string& pathSegment);
    /// TODO: fixme
    ///Builder& AddPathSegments(const std::string& pathSegment);

    Builder& SetEncodedPathSegment(int index, const std::string& pathSegment);

    /// TODO: fixme
    ///Builder& SetPathSegment(int index, const std::string& pathSegment);

    Builder& RemovePathSegment(int index);

    Builder& AddEncodedQueryParameter(const std::string& name, const std::string& value);

    Builder& RemoveEncodedQueryParameter(const std::string& name, const std::string& value);

    Builder& RemoveEncodedQueryParameter(const std::string& name);

    Builder& AddEncodedFragment(const std::string& fragment);

    HttpUrl Build();

    bool Parse(std::string url);

    std::string ToString() const;


  private:
    void Pop();

    void Push(const std::string& segment);

    void Clear() {
      scheme_.clear();
      host_.clear();
      port_ = -1;
      encoded_path_segments_.clear();
      encoded_query_names_and_values_.clear();
      encoded_fragment_.clear();
    }
  private:
    friend class HttpUrl;
    std::string scheme_;
    std::string host_;
    int port_{-1};
    std::list<std::string> encoded_path_segments_;
    std::map<std::string, std::set<std::string>> encoded_query_names_and_values_;
    std::string encoded_fragment_;

  };

public:

  explicit HttpUrl(const Builder& builder);

  explicit HttpUrl(std::string url);

  HttpUrl(const HttpUrl& url);

  std::string Url();

  std::string Scheme();

  int Port();

  bool IsHttps();

  std::string Host();

  int PathSize();

  int QuerySize();

  std::set<std::string> QueryParameter();

  std::string EncodedPath();

  std::string EncodedQuery();

  std::string EncodedFragment();

  /// TODO: return decoded string

  static int DefaultPort(const std::string& scheme);

private:
  void Init(const Builder& builder);

private:
  /** Canonical URL. */
  std::string url_;

  /** http or https */
  std::string scheme_;
  /** Either 80, 443 or a user-specified port. In range [1..65535]. */
  int port_;
  /** Canonical hostname */
  std::string host_;
  /** Decoded fragment. */
  std::string fragment_;
  /** Decoded segments. */
  std::list<std::string> path_segments_;
  /** Decoded query. */
  std::map<std::string, std::set<std::string>> query_names_and_values_;

};

}

#endif //LICODECPPCLIENT_HTTP_URL_H
