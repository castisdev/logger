#pragma once

#include <sys/syscall.h>

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <memory>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include <boost/log/sinks/async_frontend.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
// https://github.com/fmtlib/fmt
#include "fmt/format.h"

constexpr const char* cilogger_str_end(const char* str) {
  return *str ? cilogger_str_end(str + 1) : str;
}
constexpr bool cilogger_str_slant(const char* str) {
  return *str == '/' ? true : (*str ? cilogger_str_slant(str + 1) : false);
}
constexpr const char* cilogger_r_slant(const char* str) {
  return *str == '/' ? (str + 1) : cilogger_r_slant(str - 1);
}
constexpr const char* cilogger_file_name(const char* str) {
  return cilogger_str_slant(str) ? cilogger_r_slant(cilogger_str_end(str))
                                 : str;
}

#define CASTIS_CILOG_DEFAULT_MODULUE "default"

#define CIMLOG(...)                                                   \
  BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 2), \
              CIMLOG_2, CIMLOG_3)                                     \
  (__VA_ARGS__)

#define CIMLOG_2(module_name, severity)                              \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), #module_name, severity) \
      << cilogger_file_name(__FILE__) << "::" << __FUNCTION__ << ":" \
      << __LINE__ << ":" << syscall(SYS_gettid) << "," << #module_name << ","

#define CIMLOG_3(module_name, severity, fmt_str, ...)                         \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), #module_name, severity)          \
      << cilogger_file_name(__FILE__) << "::" << __FUNCTION__ << ":"          \
      << __LINE__ << ":" << syscall(SYS_gettid) << "," << #module_name << "," \
      << fmt::format(FMT_STRING(fmt_str), ##__VA_ARGS__)

#define CILOG(...)                                                             \
  BOOST_PP_IF(BOOST_PP_EQUAL(BOOST_PP_VARIADIC_SIZE(__VA_ARGS__), 1), CILOG_1, \
              CILOG_2)                                                         \
  (__VA_ARGS__)

#define CILOG_1(severity)                                                  \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), CASTIS_CILOG_DEFAULT_MODULUE, \
                        severity)                                          \
      << cilogger_file_name(__FILE__) << "::" << __FUNCTION__ << ":"       \
      << __LINE__ << ":" << syscall(SYS_gettid) << ",,"

#define CILOG_2(severity, fmt_str, ...)                                    \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), CASTIS_CILOG_DEFAULT_MODULUE, \
                        severity)                                          \
      << cilogger_file_name(__FILE__) << "::" << __FUNCTION__ << ":"       \
      << __LINE__ << ":" << syscall(SYS_gettid) << ",,"                    \
      << fmt::format(FMT_STRING(fmt_str), ##__VA_ARGS__)

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
  std::ofstream file_;
  std::filesystem::path target_path_;
  std::filesystem::path file_path_;
  std::string file_name_suffix_;
  std::string file_name_prefix_format_;
  std::string current_date_hour_;

 public:
  cilog_date_hour_backend(std::filesystem::path const& target_path,
                          std::string_view file_name_suffix,
                          std::string_view file_name_prefix_format,
                          bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               string_type const& formatted_message);

 private:
  void rotate_file();
  std::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string_view format);
  std::string get_current_date_hour();
};

class cilog_backend : public boost::log::sinks::basic_formatted_sink_backend<
                          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_{};
  std::ofstream file_;
  std::filesystem::path target_path_;
  std::filesystem::path file_path_;
  std::string file_name_suffix_;
  uintmax_t rotation_size_{};
  uintmax_t characters_written_{};
  boost::gregorian::date current_date_;

 public:
  cilog_backend(std::filesystem::path const& target_path,
                std::string_view file_name_suffix, uintmax_t rotation_size,
                bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               string_type const& formatted_message);

 private:
  void rotate_file();
  std::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string_view format);
  uintmax_t scan_next_index(std::filesystem::path const& path,
                            std::regex const& pattern);
  uintmax_t parse_index(std::string const& filename);
};

namespace castis {
namespace logger {
using cilog_sync_sink_t = boost::log::sinks::synchronous_sink<cilog_backend>;
using cilog_async_sink_t = boost::log::sinks::asynchronous_sink<cilog_backend>;
using cilog_date_hour_async_sink_t =
    boost::log::sinks::asynchronous_sink<cilog_date_hour_backend>;

void init_logger(std::string app_name, std::string app_version,
                 std::string_view target = "./log",
                 int64_t rotation_size = 10 * 1024 * 1024,
                 bool auto_flush = true);

boost::shared_ptr<cilog_async_sink_t> init_async_logger(
    std::string app_name, std::string app_version,
    std::string_view target = "./log", int64_t rotation_size = 10 * 1024 * 1024,
    bool auto_flush = true);

struct Module {
  enum { min_level, specific_level };

  Module(std::string_view name, severity_level level)
      : name_(name), level_type_(Module::min_level), min_level_(level) {}
  Module(std::string_view name, std::set<severity_level> specific_levels)
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
    std::string app_name, std::string app_version,
    const std::vector<Module>& filters, std::string_view file_name_suffix,
    std::string_view target = "./log", int64_t rotation_size = 10 * 1024 * 1024,
    bool auto_flush = true);

bool func_module_ptr_severity_filter(
    boost::log::value_ref<std::string> const& ch,
    boost::log::value_ref<severity_level> const& level,
    const std::vector<std::shared_ptr<Module>>& modules);

boost::shared_ptr<cilog_async_sink_t> init_async_module_logger(
    std::string app_name, std::string app_version,
    const std::vector<std::shared_ptr<Module>>& filters,
    std::string_view file_name_suffix, std::string_view target = "./log",
    int64_t rotation_size = 10 * 1024 * 1024, bool auto_flush = true);

boost::shared_ptr<cilog_date_hour_async_sink_t> init_async_date_hour_logger(
    std::string app_name, std::string app_version,
    std::string_view target = "./log",
    std::string_view file_name_prefix_format = "{:%Y-%m-%d[%H]}",
    bool auto_flush = true);

bool func_severity_filter(boost::log::value_ref<severity_level> const& level,
                          const std::vector<severity_level>& severity_levels);

boost::shared_ptr<cilog_async_sink_t> init_async_level_logger(
    std::string app_name, std::string app_version,
    const std::vector<severity_level> severity_levels,
    std::string_view file_name_suffix, std::string_view target = "./log",
    int64_t rotation_size = 10 * 1024 * 1024, bool auto_flush = true);

boost::shared_ptr<cilog_date_hour_async_sink_t>
init_async_date_hour_level_logger(
    std::string app_name, std::string app_version,
    const std::vector<severity_level> severity_levels,
    std::string_view file_name_suffix, std::string_view target = "./log",
    std::string_view file_name_prefix_format = "{:%Y-%m-%d[%H]}",
    bool auto_flush = true);

template <typename Sink>
void stop_logger(Sink sink) {
  auto core = boost::log::core::get();
  core->remove_sink(sink);
  sink->stop();
  sink->flush();
  sink.reset();
}

}  // namespace logger
}  // namespace castis

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    my_logger, boost::log::sources::severity_logger_mt<severity_level>)

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(
    ChanelLogger,
    boost::log::sources::severity_channel_logger_mt<severity_level>)
