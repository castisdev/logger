#include "logger/castisaccesslogger.h"

#include <boost/log/expressions.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>

namespace castis {
namespace logger {
namespace accesslog {
const std::string kDelimiter = " ";
const std::string kQuotes = "\"";
const std::string kEmpty = "-";
std::string request_line(boost::string_view method, boost::string_view uri,
                         unsigned version_major /* = 1*/,
                         unsigned version_minor /* = 1*/) {
  return fmt::format(FMT_STRING("{}{}{}{}HTTP/{}.{}"),
                     std::string_view(method.data(), method.length()),
                     kDelimiter, std::string_view(uri.data(), uri.length()),
                     kDelimiter, version_major, version_minor);
}

std::uint64_t serve_duration(boost::posix_time::ptime req_utc) {
  auto res_utc = boost::posix_time::microsec_clock::universal_time();
  auto duration = res_utc - req_utc;
  std::uint64_t du = duration.total_milliseconds();
  return du;
}

std::string request_time(boost::posix_time::ptime req_utc) {
  std::time_t utct = boost::posix_time::to_time_t(req_utc);
  struct tm nowtm;
  localtime_r(&utct, &nowtm);
  char mbstr[64];
  std::string request_time_str = accesslog::kEmpty;
  std::size_t n =
      std::strftime(mbstr, sizeof(mbstr), "[%d/%b/%Y:%H:%M:%S %z]", &nowtm);
  if (n > 0) request_time_str.assign(mbstr, n);
  return request_time_str;
}
}  // namespace accesslog

AccessLog::AccessLog(boost::string_view remote_addr,
                     boost::string_view remote_ident,
                     boost::string_view user_name,
                     boost::string_view request_time,
                     boost::string_view request_line, unsigned status,
                     std::size_t content_length, boost::string_view referer,
                     boost::string_view user_agent,
                     std::uint64_t serve_duration)
    : remote_addr_(remote_addr),
      remote_ident_(remote_ident),
      user_name_(user_name),
      request_time_(request_time),
      request_line_(request_line),
      status_(status),
      content_length_(content_length),
      referer_(referer),
      user_agent_(user_agent),
      serve_duration_(serve_duration) {}

// [:data] foramt -> [05/Sep/2018:16:48:09 +0900]
// ':remote-addr - - [:date] ":method :uri HTTP/:http-version" :status
// :res[content-length] ":referrer" ":user-agent"'
std::ostream& operator<<(std::ostream& lhs, const AccessLog& rhs) {
  namespace alog = castis::logger::accesslog;
  if (rhs.remote_addr_.empty())
    lhs << alog::kEmpty;
  else
    lhs << rhs.remote_addr_;
  // lhs << (rhs.remote_addr_.empty() ? alog::kEmpty : rhs.remote_addr_);
  lhs << alog::kDelimiter;

  lhs << (rhs.remote_ident_.empty() ? alog::kEmpty : rhs.remote_ident_);
  lhs << alog::kDelimiter;

  lhs << (rhs.user_name_.empty() ? alog::kEmpty : rhs.user_name_);
  lhs << alog::kDelimiter;

  lhs << (rhs.request_time_.empty() ? alog::kEmpty : rhs.request_time_);
  lhs << alog::kDelimiter;

  lhs << alog::kQuotes;
  lhs << (rhs.request_line_.empty() ? alog::kEmpty : rhs.request_line_);
  lhs << alog::kQuotes;
  lhs << alog::kDelimiter;

  if (rhs.status_ <= 0)
    lhs << alog::kEmpty;
  else
    lhs << rhs.status_;
  lhs << alog::kDelimiter;

  if (rhs.content_length_ <= 0)
    lhs << alog::kEmpty;
  else
    lhs << rhs.content_length_;
  lhs << alog::kDelimiter;

  lhs << alog::kQuotes;
  lhs << (rhs.referer_.empty() ? alog::kEmpty : rhs.referer_);
  lhs << alog::kQuotes;
  lhs << alog::kDelimiter;

  lhs << alog::kQuotes;
  lhs << (rhs.user_agent_.empty() ? alog::kEmpty : rhs.user_agent_);
  lhs << alog::kQuotes;

  return lhs;
}

boost::shared_ptr<cilog_async_sink_t> init_access_logger(
    const std::string& file_name, const std::string& target /* = "./log"*/,
    int64_t rotation_size /* = 10 * 1024 * 1024*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_backend>(
      std::filesystem::path(target), file_name, rotation_size, true);
  auto sink = boost::make_shared<cilog_async_sink_t>(backend);

  // NCSA Combined Log Format
  // https://zetawiki.com/wiki/NCSA_%EB%A1%9C%EA%B7%B8_%ED%98%95%EC%8B%9D
  sink->set_formatter(
      expr::stream
      << expr::attr<std::string, castisaccesslog_attr_tag>("remoteAddress")
      << accesslog::kDelimiter
      << expr::attr<std::string, castisaccesslog_attr_tag>("remoteIdent")
      << accesslog::kDelimiter
      << expr::attr<std::string, castisaccesslog_attr_tag>("userName")
      << accesslog::kDelimiter
      << expr::attr<std::string, castisaccesslog_attr_tag>("requestTime")
      << accesslog::kDelimiter << accesslog::kQuotes
      << expr::attr<std::string, castisaccesslog_attr_tag>("requestLine")
      << accesslog::kQuotes << accesslog::kDelimiter
      << expr::attr<unsigned, castisaccesslog_attr_tag>("status")
      << accesslog::kDelimiter
      << expr::attr<std::size_t, castisaccesslog_attr_tag>("contentLength")
      << accesslog::kDelimiter << accesslog::kQuotes
      << expr::attr<std::string, castisaccesslog_attr_tag>("referer")
      << accesslog::kQuotes << accesslog::kDelimiter << accesslog::kQuotes
      << expr::attr<std::string, castisaccesslog_attr_tag>("userAgent")
      << accesslog::kQuotes);

  sink->set_filter(expr::attr<std::string>("Channel") == "access");
  boost::log::core::get()->add_sink(sink);
  return sink;
}

}  // namespace logger
}  // namespace castis

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<unsigned, castisaccesslog_attr_tag> const& manip) {
  namespace alog = castis::logger::accesslog;
  unsigned value = manip.get();
  if (value <= 0)
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::size_t, castisaccesslog_attr_tag> const&
        manip) {
  namespace alog = castis::logger::accesslog;
  std::size_t value = manip.get();
  if (value <= 0)
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}

boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<std::string, castisaccesslog_attr_tag> const&
        manip) {
  namespace alog = castis::logger::accesslog;
  std::string value = manip.get();
  if (value.empty())
    strm << alog::kEmpty;
  else
    strm << value;
  return strm;
}
