#include <cstddef>
#include <iostream>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/manipulators/to_log.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/global_logger_storage.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;
namespace sinks = boost::log::sinks;

typedef sinks::synchronous_sink<sinks::text_file_backend> file_sink;

#define CILOG(severity)\
  BOOST_LOG_SEV(my_logger::get(), severity) << "," << boost::filesystem::path(__FILE__).filename().string() << "::" << __FUNCTION__ << ":" << __LINE__ << ",,"

// We define our own severity levels
enum severity_level
{
  foo,
  debug,
  report,
  info,
  success,
  warning,
  error,
  fail,
  exception,
  critical
};

// The operator is used for regular stream formatting
std::ostream& operator<< (std::ostream& strm, severity_level level)
{
  static const char* strings[] =
  {
    "Foo",
    "Debug",
    "Report",
    "Information",
    "Success",
    "Warming",
    "Error",
    "Fail",
    "Exception",
    "Critical"
  };

  if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
    strm << strings[level];
  else
    strm << static_cast< int >(level);

  return strm;
}

// Attribute value tag type
struct severity_tag;

// The operator is used when putting the severity level to log
logging::formatting_ostream& operator<< (logging::formatting_ostream& strm,
    logging::to_log_manip<severity_level, severity_tag> const& manip)
{
  static const char* strings[] =
  {
    "Foo",
    "Debug",
    "Report",
    "Information",
    "Success",
    "Warming",
    "Error",
    "Fail",
    "Exception",
    "Critical"
  };

  severity_level level = manip.get();
  if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
    strm << strings[level];
  else
    strm << static_cast< int >(level);

  return strm;
}

namespace castis
{
  namespace logger
  {
    static std::string app_name_;
    static std::string app_version_;
    static void reset_target(const std::string& target)
    {
      logging::core::get()->remove_all_sinks();
      boost::shared_ptr<file_sink> sink(new file_sink(
            keywords::file_name = "%Y-%m-%d_" + app_name_ + "_%N.log",
            keywords::rotation_size = 10 * 1024 * 1024, /*< rotate files every 10 MiB... >*/
            keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0) /*< ...or at midnight >*/
            ));
      sink->set_formatter(

          expr::stream
          << app_name_
          << "," << app_version_
          << "," << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
          << "," << expr::attr<severity_level, severity_tag>("Severity")
          << "," << expr::smessage
          );
      sink->locked_backend()->set_file_collector(sinks::file::make_collector(
          keywords::target = target
          // keywords::max_size = 16 * 1024 * 1024,  // maximum total size of the stored files, in bytes
          // keywords::min_free_space = 100 * 1024 * 1024  // minimum free space on the drive, in bytes
          ));
      logging::core::get()->add_sink(sink);
    }
    static void init(const std::string& app_name, const std::string& app_version)
    {
      app_name_ = app_name;
      app_version_ = app_version;
      logging::add_common_attributes();
      reset_target("/var/log/castis/"+app_name);
    }
  }
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt<severity_level>)

