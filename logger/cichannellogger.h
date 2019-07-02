#pragma once

#include "castislogger.h"

// [[deprecated]]
// cilogger 가 channel 속성을 갖게됨으로해서, 더이상 사용할 필요가 없어짐
//
// cichannellogger.h
//
// almost logic come from castislogger.h
//
#define CILOG_CHANNEL(channel_name, severity) \
  BOOST_LOG_FUNCTION()                        \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), channel_name, severity)

#define CILOGF_CHANNEL(channel_name, severity, format, ...)          \
  BOOST_LOG_FUNCTION()                                               \
  BOOST_LOG_CHANNEL_SEV(ChanelLogger::get(), channel_name, severity) \
      << castis::logger::formatter(format, ##__VA_ARGS__)

class cilog_date_hour_based_backup_backend
    : public boost::log::sinks::basic_formatted_sink_backend<
          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_{};
  boost::filesystem::ofstream file_;
  boost::filesystem::path target_path_;
  boost::filesystem::path file_path_;
  std::string file_name_suffix_;
  std::string current_date_hour_;
  std::string file_name_prefix_format_;

 public:
  cilog_date_hour_based_backup_backend(
      boost::filesystem::path const& target_path,
      std::string const& file_name_suffix,
      std::string const& file_name_prefix_format, bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               std::basic_string<char> const& formatted_message);

 private:
  void rotate_file();
  boost::filesystem::path generate_active_filepath();
  void rollover_without_max();
  boost::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string const& format, int hour);
  std::string get_current_date_hour();
};

// cilog_size_based_backup_backend :
// change: backup file begin index : 0 -> 1
// almost logics come from cilog_backend
//
// log file name example:
// active log file :
// example.log
// backup log file :
// 2018-10/2018-10-12[1]_example.log
// 2018-10/2018-10-12[2]_example.log
//
// WARN : crash when log path set to current directory {"."}.
// fiexed : Fix #12495 BOOST-1.68.0
class cilog_size_based_backup_backend
    : public boost::log::sinks::basic_formatted_sink_backend<
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
  const unsigned int kBackupBeginIndex{1};

 public:
  cilog_size_based_backup_backend(boost::filesystem::path const& target_path,
                                  std::string const& file_name_suffix,
                                  uintmax_t rotation_size, bool auto_flush);
  void consume(boost::log::record_view const& /*rec*/,
               std::basic_string<char> const& formatted_message);

 private:
  void rotate_file();
  boost::filesystem::path generate_active_filepath();
  void rollover_without_max();
  boost::filesystem::path generate_filepath();
  std::string datetime_string_with_format(std::string const& format);
  uintmax_t scan_next_index(boost::filesystem::path const& path,
                            boost::regex const& pattern);
  uintmax_t parse_index(std::string const& filename);
};

namespace castis {
namespace logger {
using cichannellog_async_sink =
    boost::log::sinks::asynchronous_sink<cilog_size_based_backup_backend>;
using cichannellog_date_hour_async_sink =
    boost::log::sinks::asynchronous_sink<cilog_date_hour_based_backup_backend>;

boost::shared_ptr<cichannellog_async_sink> init_async_channel_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool with_func_scope = false,
    bool csv_format = true, bool auto_flush = true);

boost::shared_ptr<cichannellog_date_hour_async_sink>
init_async_date_hour_channel_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name, const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool with_func_scope = false, bool csv_format = true,
    bool auto_flush = true);

boost::shared_ptr<cichannellog_async_sink> init_async_channel_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool with_func_scope = false,
    bool csv_format = true, bool auto_flush = true);

boost::shared_ptr<cichannellog_date_hour_async_sink>
init_async_date_hour_channel_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool with_func_scope = false, bool csv_format = true,
    bool auto_flush = true);
}  // namespace logger
}  // namespace castis
