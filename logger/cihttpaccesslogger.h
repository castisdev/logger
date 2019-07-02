#pragma once

#include "cichannellogger.h"

//
// [[deprecated]]
// castishttpaccesslogger.h 로 대체됨
//
// cihttpaccesslogger.h
//
#define HTTPACCESSLOG(http_access)                                        \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), "ACCESS", info)              \
      << boost::log::add_value("remoteAddress", http_access.remote_addr)  \
      << boost::log::add_value("remoteIdent", http_access.remote_ident)   \
      << boost::log::add_value("userName", http_access.user_name)         \
      << boost::log::add_value("requestTime", http_access.request_time)   \
      << boost::log::add_value("requestLine", http_access.request_line)   \
      << boost::log::add_value("status",                                  \
                               static_cast<unsigned>(http_access.status)) \
      << boost::log::add_value(                                           \
             "contentLength",                                             \
             static_cast<std::size_t>(http_access.content_length))        \
      << boost::log::add_value("referer", http_access.referer)            \
      << boost::log::add_value("userAgent", http_access.user_agent)

struct cihttpaccesslog_attr_tag;

namespace castis {
namespace logger {
namespace httpaccesslog {
std::string request_line(const std::string& method, const std::string& uri,
                         unsigned version_major = 1,
                         unsigned version_minor = 1);
std::string request_time();
}  // namespace httpaccesslog

struct HttpAccess {          // https://developer.mozilla.org/ko/docs/Web/HTTP
  std::string remote_addr;   // Remote client ip
  std::string remote_ident;  // REMOTE_IDENT : The remote logname, user id
  std::string user_name;     // The name of the authenticated remote user
  std::string request_time;  // Request time : Date and time of the request.
  std::string
      request_line;  // The first line of the request. Example: GET / HTTP/1.0
  unsigned status;   // Final status code of http response
  std::size_t content_length;  // Content-Length of http response
  std::string referer;         // Referer of http req header
  std::string user_agent;      // User-Agent of http req heder

  // [:data] foramt -> [05/Sep/2018:16:48:09 +0900]
  // ':remote-addr - - [:date] ":method :uri HTTP/:http-version" :status
  // :res[content-length] ":referrer" ":user-agent"'
  std::string to_string() const;

  friend std::ostream& operator<<(std::ostream& os,
                                  const HttpAccess& http_access);
};

std::ostream& operator<<(std::ostream& strm, const HttpAccess& http_access);

boost::shared_ptr<cichannellog_async_sink> init_httpaccess_logger(
    const std::string& file_name, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024);

}  // namespace logger
}  // namespace castis

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<unsigned, cihttpaccesslog_attr_tag> const& manip);

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::size_t, cihttpaccesslog_attr_tag> const&
        manip);

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::string, cihttpaccesslog_attr_tag> const&
        manip);
