#include "logger/cihttpaccesslogger.h"

#include <boost/filesystem.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace castis {
namespace logger {
namespace httpaccesslog {
const std::string kDelimiter = " ";
const std::string kQuotes = "\"";
const std::string kEmpty = "-";

std::string request_line(const std::string& method, const std::string& uri,
                         unsigned version_major /* = 1*/,
                         unsigned version_minor /* = 1*/) {
  return method + httpaccesslog::kDelimiter + uri + httpaccesslog::kDelimiter +
         "HTTP/" + std::to_string(version_major) + "." +
         std::to_string(version_minor);
}

std::string request_time() {
  std::time_t req_t = std::time(nullptr);
  struct tm nowtm;
  localtime_r(&req_t, &nowtm);
  char mbstr[64];
  std::string request_time_str = httpaccesslog::kEmpty;
  std::size_t n =
      std::strftime(mbstr, sizeof(mbstr), "[%d/%b/%Y:%H:%M:%S %z]", &nowtm);
  if (n > 0) request_time_str.assign(mbstr, n);
  return request_time_str;
}
}  // namespace httpaccesslog

// [:data] foramt -> [05/Sep/2018:16:48:09 +0900]
// ':remote-addr - - [:date] ":method :uri HTTP/:http-version" :status
// :res[content-length] ":referrer" ":user-agent"'
std::string HttpAccess::to_string() const {
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
      (http_access.user_agent.empty() ? alog::kEmpty : http_access.user_agent) +
      alog::kQuotes;
  return str;
}

std::ostream& operator<<(std::ostream& strm, const HttpAccess& http_access) {
  strm << http_access.to_string();
  return strm;
}

boost::shared_ptr<cichannellog_async_sink> init_httpaccess_logger(
    const std::string& file_name, const std::string& target /* = "./log"*/,
    int64_t rotation_size /* = 100 * 100 * 1024*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  boost::shared_ptr<cilog_size_based_backup_backend> backend(
      new cilog_size_based_backup_backend(boost::filesystem::path(target),
                                          file_name, rotation_size, true));

  boost::shared_ptr<cichannellog_async_sink> sink(
      new cichannellog_async_sink(backend));

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

  sink->set_filter(expr::attr<std::string>("Channel") == "ACCESS");
  boost::log::core::get()->add_sink(sink);
  return sink;
}

}  // namespace logger
}  // namespace castis

boost::log::formatting_ostream& operator<<(
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

boost::log::formatting_ostream& operator<<(
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

boost::log::formatting_ostream& operator<<(
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
