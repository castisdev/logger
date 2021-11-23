#pragma once

#include <string_view>

#include <boost/log/utility/manipulators/add_value.hpp>

#include "castislogger.h"

#define ACCESSLOG(accesslog)                                             \
  CIMLOG(access, info)                                                   \
      << boost::log::add_value("remoteAddress", accesslog.remote_addr_)  \
      << boost::log::add_value("remoteIdent", accesslog.remote_ident_)   \
      << boost::log::add_value("userName", accesslog.user_name_)         \
      << boost::log::add_value("requestTime", accesslog.request_time_)   \
      << boost::log::add_value("requestLine", accesslog.request_line_)   \
      << boost::log::add_value("status",                                 \
                               static_cast<unsigned>(accesslog.status_)) \
      << boost::log::add_value(                                          \
             "contentLength",                                            \
             static_cast<std::size_t>(accesslog.content_length_))        \
      << boost::log::add_value("referer", accesslog.referer_)            \
      << boost::log::add_value("userAgent", accesslog.user_agent_)       \
      << boost::log::add_value(                                          \
             "serveDuration",                                            \
             static_cast<std::uint64_t>(accesslog.serve_duration_))      \
      << accesslog;

struct castisaccesslog_attr_tag;

namespace castis {
namespace logger {
namespace accesslog {

std::string request_line(std::string_view method, std::string_view uri,
                         unsigned version_major = 1,
                         unsigned version_minor = 1);
std::uint64_t serve_duration(boost::posix_time::ptime req_utc);
std::string request_time(boost::posix_time::ptime req_utc);
}  // namespace accesslog

// https://developer.mozilla.org/ko/docs/Web/HTTP
// https://httpd.apache.org/docs/2.4/en/logs.html
// https://httpd.apache.org/docs/2.2/en/mod/mod_log_config.html#formats
struct AccessLog {
  AccessLog(std::string_view remote_addr, std::string_view remote_ident,
            std::string_view user_name, std::string_view request_time,
            std::string_view request_line, unsigned status,
            std::size_t content_length, std::string_view referer,
            std::string_view user_agent, std::uint64_t serve_duration);

  // Remote client ip : %h
  std::string remote_addr_;
  // REMOTE_IDENT : The remote logname, user id : %l
  std::string remote_ident_;
  // The name of the authenticated remote user : %u
  std::string user_name_;
  // Request time : Date and time of the request : %t
  std::string request_time_;
  // The first line of the request. Example: GET / HTTP/1.0 : %r
  std::string request_line_;
  // Final status code of http response : %>s
  unsigned status_;
  // Content-Length of http response : %b
  std::size_t content_length_;
  // Referer of http req header : %{Referer}i
  std::string referer_;
  // User-Agent of http req heder : %{User-agent}i
  std::string user_agent_;
  // The time taken to serve the request, in microseconds : %D
  std::uint64_t serve_duration_;

  // [:data] foramt -> [05/Sep/2018:16:48:09 +0900]
  // ':remote-addr - - [:date] ":method :uri HTTP/:http-version" :status
  // :res[content-length] ":referrer" ":user-agent"'
  friend std::ostream& operator<<(std::ostream& os, const AccessLog& accesslog);
};

std::ostream& operator<<(std::ostream& strm, const AccessLog& accesslog);

boost::shared_ptr<cilog_async_sink_t> init_access_logger(
    const std::string& file_name, const std::string& target = "./log",
    int64_t rotation_size = 10 * 1024 * 1024);

}  // namespace logger
}  // namespace castis

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<unsigned, castisaccesslog_attr_tag> const& manip);

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::size_t, castisaccesslog_attr_tag> const&
        manip);

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::string, castisaccesslog_attr_tag> const&
        manip);
