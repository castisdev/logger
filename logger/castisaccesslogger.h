#pragma once
#include <boost/log/utility/manipulators/add_value.hpp>
#include "castislogger.h"

#define ACCESSLOG(http_access)                                              \
  CIMLOG(access, info)                                                      \
      << boost::log::add_value("remoteAddress", http_access.remote_addr)    \
      << boost::log::add_value("remoteIdent", http_access.remote_ident)     \
      << boost::log::add_value("userName", http_access.user_name)           \
      << boost::log::add_value("requestTime", http_access.request_time)     \
      << boost::log::add_value("requestLine", http_access.request_line)     \
      << boost::log::add_value("status",                                    \
                               static_cast<unsigned>(http_access.status))   \
      << boost::log::add_value(                                             \
             "contentLength",                                               \
             static_cast<std::size_t>(http_access.content_length))          \
      << boost::log::add_value("referer", http_access.referer)              \
      << boost::log::add_value("userAgent", http_access.user_agent)         \
      << boost::log::add_value("serveDuration", http_access.serve_duration) \
      << http_access.to_string();

struct cihttpaccesslog_attr_tag;

namespace castis {
namespace logger {

namespace httpaccesslog {

const std::string kDelimiter = " ";
const std::string kQuotes = "\"";
const std::string kEmpty = "-";
static std::string request_line(const std::string& method,
                                const std::string& uri,
                                unsigned version_major = 1,
                                unsigned version_minor = 1) {
  return method + httpaccesslog::kDelimiter + uri + httpaccesslog::kDelimiter +
         "HTTP/" + std::to_string(version_major) + "." +
         std::to_string(version_minor);
}
static std::uint64_t serve_duration(boost::posix_time::ptime req_utc) {
  auto res_utc = boost::posix_time::microsec_clock::universal_time();
  auto duration = res_utc - req_utc;
  std::uint64_t du = duration.total_milliseconds();
  return du;
}
static std::string request_time(boost::posix_time::ptime req_utc) {
  std::time_t utct = boost::posix_time::to_time_t(req_utc);
  struct tm nowtm;
  localtime_r(&utct, &nowtm);
  char mbstr[64];
  std::string request_time_str = httpaccesslog::kEmpty;
  std::size_t n =
      std::strftime(mbstr, sizeof(mbstr), "[%d/%b/%Y:%H:%M:%S %z]", &nowtm);
  if (n > 0) request_time_str.assign(mbstr, n);
  return request_time_str;
}
}  // namespace httpaccesslog

// https://developer.mozilla.org/ko/docs/Web/HTTP
// https://httpd.apache.org/docs/2.4/en/logs.html
// https://httpd.apache.org/docs/2.2/en/mod/mod_log_config.html#formats
struct HttpAccess {
  // Remote client ip : %h
  std::string remote_addr;
  // REMOTE_IDENT : The remote logname, user id : %l
  std::string remote_ident;
  // The name of the authenticated remote user : %u
  std::string user_name;
  // Request time : Date and time of the request : %t
  std::string request_time;
  // The first line of the request. Example: GET / HTTP/1.0 : %r
  std::string request_line;
  // Final status code of http response : %>s
  unsigned status;
  // Content-Length of http response : %b
  std::size_t content_length;
  // Referer of http req header : %{Referer}i
  std::string referer;
  // User-Agent of http req heder : %{User-agent}i
  std::string user_agent;
  // The time taken to serve the request, in microseconds : %D
  std::uint64_t serve_duration;

  // [:data] foramt -> [05/Sep/2018:16:48:09 +0900]
  // ':remote-addr - - [:date] ":method :uri HTTP/:http-version" :status
  // :res[content-length] ":referrer" ":user-agent"'
  std::string to_string() const {
    namespace alog = castis::logger::httpaccesslog;
    const HttpAccess& http_access = *this;
    std::string str =
        (http_access.remote_addr.empty() ? alog::kEmpty
                                         : http_access.remote_addr) +
        alog::kDelimiter +
        (http_access.remote_ident.empty() ? alog::kEmpty
                                          : http_access.remote_ident) +
        alog::kDelimiter +
        (http_access.user_name.empty() ? alog::kEmpty : http_access.user_name) +
        alog::kDelimiter +
        (http_access.request_time.empty() ? alog::kEmpty
                                          : http_access.request_time) +
        alog::kDelimiter + alog::kQuotes +
        (http_access.request_line.empty() ? alog::kEmpty
                                          : http_access.request_line) +
        alog::kQuotes + alog::kDelimiter +
        ((http_access.status <= 0) ? alog::kEmpty
                                   : std::to_string(http_access.status)) +
        alog::kDelimiter +
        ((http_access.content_length <= 0)
             ? alog::kEmpty
             : std::to_string(http_access.content_length)) +
        alog::kDelimiter + alog::kQuotes +
        (http_access.referer.empty() ? alog::kEmpty : http_access.referer) +
        alog::kQuotes + alog::kDelimiter + alog::kQuotes +
        (http_access.user_agent.empty() ? alog::kEmpty
                                        : http_access.user_agent) +
        alog::kQuotes;
    return str;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const HttpAccess& http_access);
};

inline std::ostream& operator<<(std::ostream& strm,
                                const HttpAccess& http_access) {
  strm << http_access.to_string();
  return strm;
}

}  // namespace logger
}  // namespace castis

inline boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<unsigned, cihttpaccesslog_attr_tag> const& manip) {
  namespace alog = castis::logger::httpaccesslog;
  unsigned value = manip.get();
  if (value <= 0)
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}
inline boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::size_t, cihttpaccesslog_attr_tag> const&
        manip) {
  namespace alog = castis::logger::httpaccesslog;
  std::size_t value = manip.get();
  if (value <= 0)
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}
inline boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::string, cihttpaccesslog_attr_tag> const&
        manip) {
  namespace alog = castis::logger::httpaccesslog;
  std::string value = manip.get();
  if (value.empty())
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}

namespace castis {
namespace logger {

inline boost::shared_ptr<cilog_async_sink_t> init_httpaccess_logger(
    const std::string& file_name, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_backend>(
      boost::filesystem::path(target), file_name, rotation_size, true);
  auto sink = boost::make_shared<cilog_async_sink_t>(backend);

  // NCSA Combined Log Format
  // https://zetawiki.com/wiki/NCSA_%EB%A1%9C%EA%B7%B8_%ED%98%95%EC%8B%9D
  sink->set_formatter(
      expr::stream
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("remoteAddress")
      << httpaccesslog::kDelimiter
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("remoteIdent")
      << httpaccesslog::kDelimiter
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("userName")
      << httpaccesslog::kDelimiter
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("requestTime")
      << httpaccesslog::kDelimiter << httpaccesslog::kQuotes
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("requestLine")
      << httpaccesslog::kQuotes << httpaccesslog::kDelimiter
      << expr::attr<unsigned, cihttpaccesslog_attr_tag>("status")
      << httpaccesslog::kDelimiter
      << expr::attr<std::size_t, cihttpaccesslog_attr_tag>("contentLength")
      << httpaccesslog::kDelimiter << httpaccesslog::kQuotes
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("referer")
      << httpaccesslog::kQuotes << httpaccesslog::kDelimiter
      << httpaccesslog::kQuotes
      << expr::attr<std::string, cihttpaccesslog_attr_tag>("userAgent")
      << httpaccesslog::kQuotes);

  sink->set_filter(expr::attr<std::string>("Channel") == "access");
  boost::log::core::get()->add_sink(sink);
  return sink;
}
}  // namespace logger
}  // namespace castis
