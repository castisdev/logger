#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>
#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/regex_fwd.hpp>

#define CASTIS_CILOG_DEFAULT_MODULUE "default"

#define CIMLOG(...)                                                   \
  BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 2), \
              CIMLOG_2, CIMLOG_3)                                     \
  (__VA_ARGS__)

#define CIMLOG_2(module_name, severity)                              \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), #module_name, severity) \
      << boost::filesystem::path(__FILE__).filename().string()       \
      << "::" << __FUNCTION__ << ":" << __LINE__ << "," << #module_name << ","

#define CIMLOG_3(module_name, severity, format, ...)                           \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), #module_name, severity)           \
      << boost::filesystem::path(__FILE__).filename().string()                 \
      << "::" << __FUNCTION__ << ":" << __LINE__ << "," << #module_name << "," \
      << castis::logger::formatter(format, ##__VA_ARGS__)

#define CILOG(...)                                                             \
  BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1), CILOG_1, \
              CILOG_2)                                                         \
  (__VA_ARGS__)

#define CILOG_1(severity)                                                  \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), CASTIS_CILOG_DEFAULT_MODULUE, \
                        severity)                                          \
      << boost::filesystem::path(__FILE__).filename().string()             \
      << "::" << __FUNCTION__ << ":" << __LINE__ << ",,"

#define CILOG_2(severity, format, ...)                                     \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), CASTIS_CILOG_DEFAULT_MODULUE, \
                        severity)                                          \
      << boost::filesystem::path(__FILE__).filename().string()             \
      << "::" << __FUNCTION__ << ":" << __LINE__ << ",,"                   \
      << castis::logger::formatter(format, ##__VA_ARGS__)

enum severity_level {
  foo,
  debug,
  report,
  info,
  success,
  warning,
  error,
  fail,
  exception,
  critical,
};

struct severity_tag;
// The operator is used when putting the severity level to log
boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<severity_level, severity_tag> const& manip);

class cilog_date_hour_backend
    : public boost::log::sinks::basic_formatted_sink_backend<
          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_{};
  boost::filesystem::ofstream file_;
  boost::filesystem::path target_path_;
  boost::filesystem::path file_path_;
  std::string file_name_suffix_;
  std::string file_name_prefix_format_;
  std::string current_date_hour_;

 public:
  cilog_date_hour_backend(boost::filesystem::path const& target_path,
                          std::string const& file_name_suffix,
                          std::string const& file_name_prefix_format,
                          bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               string_type const& formatted_message);

 private:
  void rotate_file();
  boost::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string const& format);
  std::string get_current_date_hour();
};

class cilog_backend : public boost::log::sinks::basic_formatted_sink_backend<
                          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_{};
  boost::filesystem::ofstream file_;
  boost::filesystem::path target_path_;
  boost::filesystem::path file_path_;
  std::string file_name_suffix_;
  uintmax_t rotation_size_{};
  uintmax_t characters_written_{};
  boost::gregorian::date current_date_;

 public:
  cilog_backend(boost::filesystem::path const& target_path,
                std::string const& file_name_suffix, uintmax_t rotation_size,
                bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               string_type const& formatted_message);

 private:
  void rotate_file();
  boost::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string const& format);
  uintmax_t scan_next_index(boost::filesystem::path const& path,
                            boost::regex const& pattern);
  uintmax_t parse_index(std::string const& filename);
};

namespace castis {
namespace logger {
using cilog_sync_sink_t = boost::log::sinks::synchronous_sink<cilog_backend>;
using cilog_async_sink_t = boost::log::sinks::asynchronous_sink<cilog_backend>;
using cilog_date_hour_async_sink_t =
    boost::log::sinks::asynchronous_sink<cilog_date_hour_backend>;

void init_logger(const std::string& app_name, const std::string& app_version,
                 const std::string& target = "./log",
                 int64_t rotation_size = 100 * 100 * 1024,
                 bool auto_flush = true);

boost::shared_ptr<cilog_async_sink_t> init_async_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool auto_flush = true);

struct Module {
  enum { min_level, specific_level };

  Module(const std::string& name, severity_level level)
      : name_(name), level_type_(Module::min_level), min_level_(level) {}
  Module(const std::string& name, std::set<severity_level> specific_levels)
      : name_(name),
        level_type_(Module::specific_level),
        specific_levels_(specific_levels) {}
  Module(severity_level level)
      : level_type_(Module::min_level), min_level_(level) {}
  Module(std::set<severity_level> specific_levels)
      : level_type_(Module::specific_level),
        specific_levels_(specific_levels) {}

  std::string name_;
  int level_type_{min_level};
  severity_level min_level_{info};
  std::set<severity_level> specific_levels_;
};

bool func_module_severity_filter(
    boost::log::value_ref<std::string> const& ch,
    boost::log::value_ref<severity_level> const& level,
    const std::vector<Module>& modules);

boost::shared_ptr<cilog_async_sink_t> init_async_module_logger(
    const std::string& app_name, const std::string& app_version,
    const std::vector<Module>& filters, const std::string& file_name_suffix,
    const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool auto_flush = true);

bool func_module_ptr_severity_filter(
    boost::log::value_ref<std::string> const& ch,
    boost::log::value_ref<severity_level> const& level,
    const std::vector<std::shared_ptr<Module>>& modules);

boost::shared_ptr<cilog_async_sink_t> init_async_module_logger(
    const std::string& app_name, const std::string& app_version,
    const std::vector<std::shared_ptr<Module>>& filters,
    const std::string& file_name_suffix, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool auto_flush = true);

boost::shared_ptr<cilog_date_hour_async_sink_t> init_async_date_hour_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool auto_flush = true);

bool func_severity_filter(boost::log::value_ref<severity_level> const& level,
                          const std::vector<severity_level>& severity_levels);

boost::shared_ptr<cilog_async_sink_t> init_async_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool auto_flush = true);

boost::shared_ptr<cilog_date_hour_async_sink_t>
init_async_date_hour_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool auto_flush = true);

template <typename Sink>
void stop_logger(Sink sink) {
  auto core = boost::log::core::get();
  core->remove_sink(sink);
  sink->stop();
  sink->flush();
  sink.reset();
}

boost::format formatter_r(boost::format& format);

template <typename T, typename... Params>
inline boost::format formatter_r(boost::format& format, T arg,
                                 Params... parameters) {
  return formatter_r(format % arg, parameters...);
}

boost::format formatter(const char* const format);

template <typename T, typename... Params>
inline boost::format formatter(const char* const format, T arg,
                               Params... parameters) {
  return formatter_r(boost::format(format) % arg, parameters...);
}
}  // namespace logger
}  // namespace castis

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    my_logger, boost::log::sources::severity_logger_mt<severity_level>)

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    ChanelLogger,
    boost::log::sources::severity_channel_logger_mt<severity_level>)
