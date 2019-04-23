#pragma once
#include <boost/log/attributes/named_scope.hpp>
#include <boost/log/expressions/formatters/csv_decorator.hpp>
#include <boost/log/expressions/formatters/named_scope.hpp>
#include <boost/log/sources/severity_channel_logger.hpp>
#include "boost/date_time/posix_time/posix_time.hpp"
#include "castislogger.h"

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

// cilog_size_based_backup_backend :
//
// almost logics come from cilog_backend
//
// log file name example:
// active log file :
// example.log
// backup log file :
// 2018-10/2018-10-12[0]_example.log
// 2018-10/2018-10-12[1]_example.log
//
// WARN : crash when log path set to current directory {"."}.
// fiexed : Fix #12495 BOOST-1.68.0

class cilog_date_hour_based_backup_backend
    : public boost::log::sinks::basic_formatted_sink_backend<
          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_;
  boost::filesystem::ofstream file_;
  boost::filesystem::path target_path_;
  boost::filesystem::path file_path_;
  std::string file_name_suffix_;
  std::string current_date_hour_;
  std::string file_name_prefix_format_;

 public:
  explicit cilog_date_hour_based_backup_backend(
      boost::filesystem::path const& target_path,
      std::string const& file_name_suffix,
      std::string const& file_name_prefix_format, bool auto_flush)
      : auto_flush_(auto_flush),
        target_path_(target_path),
        file_name_suffix_(file_name_suffix),
        current_date_hour_(get_current_date_hour()),
        file_name_prefix_format_(file_name_prefix_format) {}

  void consume(boost::log::record_view const& /*rec*/,
               std::basic_string<char> const& formatted_message) {
    if (current_date_hour_ != get_current_date_hour()) {
      file_path_ = generate_active_filepath();
      rotate_file();
    }

    if (!file_.is_open()) {
      file_path_ = generate_active_filepath();
      boost::filesystem::create_directories(file_path_.parent_path());
      file_.open(file_path_, std::ofstream::out | std::ofstream::app);
      if (!file_.is_open()) {
        // failed to open file
        return;
      }
    }
    file_.write(formatted_message.data(),
                static_cast<std::streamsize>(formatted_message.size()));
    file_.put('\n');

    if (auto_flush_) file_.flush();

    if (current_date_hour_ != get_current_date_hour()) {
      rotate_file();
    }
  }

 private:
  void rotate_file() {
    if (file_.is_open()) {
      file_.close();
      file_.clear();
    }
    rollover_without_max();
    current_date_hour_ = get_current_date_hour();
  }

  boost::filesystem::path generate_active_filepath() {
    // e.g. example.log
    std::stringstream filename_ss;
    filename_ss << file_name_suffix_;
    filename_ss << ".log";

    return boost::filesystem::path(target_path_ / filename_ss.str());
  }

  void rollover_without_max() {
    try {
      boost::filesystem::path backup_file_path = generate_filepath();
      boost::filesystem::create_directories(backup_file_path.parent_path());
      boost::filesystem::rename(file_path_, backup_file_path);
    } catch (boost::filesystem::filesystem_error&) {
    }
  }

  boost::filesystem::path generate_filepath() {
    if (file_name_prefix_format_.empty())
      file_name_prefix_format_ = "%Y-%m-%d[%H]";
    std::string filename_prefix =
        datetime_string_with_format(file_name_prefix_format_, -1);
    std::string monthly_path_name = datetime_string_with_format("%Y-%m", -1);

    boost::filesystem::path monthly_path(target_path_ / monthly_path_name);

    // e.g. 2014-08-12[1]_example.log
    std::stringstream filename_ss;
    filename_ss << filename_prefix;
    filename_ss << "_" + file_name_suffix_;
    filename_ss << ".log";

    return boost::filesystem::path(monthly_path / filename_ss.str());
  }

  std::string datetime_string_with_format(std::string const& format, int hour) {
    std::locale loc(std::cout.getloc(),
                    new boost::posix_time::time_facet(format.c_str()));
    std::stringstream ss;
    ss.imbue(loc);
    ss << boost::posix_time::second_clock::local_time() +
              boost::posix_time::hours(hour);

    return ss.str();
  }
  std::string get_current_date_hour() {
    return datetime_string_with_format("%Y-%m-%d %H", 0);
  }
};

class cilog_size_based_backup_backend
    : public boost::log::sinks::basic_formatted_sink_backend<
          char, boost::log::sinks::synchronized_feeding> {
 private:
  bool auto_flush_;
  boost::filesystem::ofstream file_;
  boost::filesystem::path target_path_;
  boost::filesystem::path file_path_;
  std::string file_name_suffix_;
  uintmax_t rotation_size_;
  uintmax_t characters_written_;
  boost::gregorian::date current_date_;

 public:
  explicit cilog_size_based_backup_backend(
      boost::filesystem::path const& target_path,
      std::string const& file_name_suffix, uintmax_t rotation_size,
      bool auto_flush)
      : auto_flush_(auto_flush),
        target_path_(target_path),
        file_name_suffix_(file_name_suffix),
        rotation_size_(rotation_size),
        characters_written_(0),
        current_date_(boost::gregorian::day_clock::local_day()) {}

  void consume(boost::log::record_view const& /*rec*/,
               std::basic_string<char> const& formatted_message) {
    if (current_date_ != boost::gregorian::day_clock::local_day()) {
      file_path_ = generate_active_filepath();
      rotate_file();
    }

    if (!file_.is_open()) {
      file_path_ = generate_active_filepath();
      boost::filesystem::create_directories(file_path_.parent_path());
      file_.open(file_path_, std::ofstream::out | std::ofstream::app);
      if (!file_.is_open()) {
        // failed to open file
        return;
      }
      characters_written_ = static_cast<std::streamoff>(file_.tellp());
    }
    file_.write(formatted_message.data(),
                static_cast<std::streamsize>(formatted_message.size()));
    file_.put('\n');
    characters_written_ += formatted_message.size() + 1;

    if (auto_flush_) file_.flush();

    if (characters_written_ >= rotation_size_) {
      rotate_file();
    }
  }

 private:
  void rotate_file() {
    if (file_.is_open()) {
      file_.close();
      file_.clear();
    }
    characters_written_ = 0;
    // create backup files
    rollover_without_max();
    current_date_ = boost::gregorian::day_clock::local_day();
  }

  boost::filesystem::path generate_active_filepath() {
    // e.g. example.log
    std::stringstream filename_ss;
    filename_ss << file_name_suffix_;
    filename_ss << ".log";

    return boost::filesystem::path(target_path_ / filename_ss.str());
  }

  void rollover_without_max() {
    try {
      boost::filesystem::path backup_file_path = generate_filepath();
      boost::filesystem::create_directories(backup_file_path.parent_path());
      boost::filesystem::rename(file_path_, backup_file_path);
    } catch (boost::filesystem::filesystem_error&) {
    }
  }

  boost::filesystem::path generate_filepath() {
    std::string filename_prefix = datetime_string_with_format("%Y-%m-%d");
    std::string monthly_path_name = datetime_string_with_format("%Y-%m");

    boost::filesystem::path monthly_path(target_path_ / monthly_path_name);
    boost::regex pattern(filename_prefix + "(\\[[0-9]+\\])?" + "_" +
                         file_name_suffix_ + ".log");
    uintmax_t next_index = scan_next_index(monthly_path, pattern);

    std::stringstream filename_ss;
    filename_ss << filename_prefix;

    filename_ss << "[" << next_index << "]";
    filename_ss << "_" + file_name_suffix_;
    filename_ss << ".log";

    return boost::filesystem::path(monthly_path / filename_ss.str());
  }

  std::string datetime_string_with_format(std::string const& format) {
    std::locale loc(std::cout.getloc(),
                    new boost::posix_time::time_facet(format.c_str()));
    std::stringstream ss;
    ss.imbue(loc);
    ss << boost::posix_time::second_clock::local_time();
    return ss.str();
  }

  uintmax_t scan_next_index(boost::filesystem::path const& path,
                            boost::regex const& pattern) {
    uintmax_t current_index = 0;
    boost::filesystem::path current_fs;
    if (boost::filesystem::exists(path) &&
        boost::filesystem::is_directory(path)) {
      boost::filesystem::directory_iterator it(path), end;
      for (; it != end; ++it) {
        if (boost::regex_match(it->path().filename().string(), pattern)) {
          uintmax_t index = parse_index(it->path().filename().string());
          if (index >= current_index) {
            current_index = index;
            current_fs = it->path();
          }
        }
      }
    }
    boost::system::error_code ec;
    uintmax_t filesize = boost::filesystem::file_size(current_fs, ec);
    if (!ec) {
      if (filesize >= rotation_size_) {
        ++current_index;
      }
    }
    return current_index;
  }

  uintmax_t parse_index(std::string const& filename) {
    size_t pos_index_begin = filename.find('[');
    size_t pos_index_end = filename.find(']');
    unsigned int index = 0;
    if (pos_index_begin != std::string::npos &&
        pos_index_end != std::string::npos) {
      index = atoi(
          filename.substr(pos_index_begin + 1, pos_index_end - pos_index_begin)
              .c_str());
    }
    return index;
  }
};

namespace castis {
namespace logger {

typedef boost::log::sinks::asynchronous_sink<cilog_size_based_backup_backend>
    cichannellog_async_sink;
typedef boost::log::sinks::asynchronous_sink<
    cilog_date_hour_based_backup_backend>
    cichannellog_date_hour_async_sink;

inline boost::shared_ptr<cichannellog_async_sink> init_async_channel_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool with_func_scope = false,
    bool csv_format = true, bool auto_flush = true) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_size_based_backup_backend>(
      boost::filesystem::path(target), app_name, rotation_size, auto_flush);

  auto sink = boost::make_shared<cichannellog_async_sink>(backend);

  std::string fs = "%F::%C:%l";
  if (with_func_scope) fs = "%F::%c:%l";
  if (csv_format) {
    sink->set_formatter(
        expr::stream
        << app_name << "," << app_version << ","
        << expr::format_date_time<boost::posix_time::ptime>(
               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
        << "," << expr::attr<severity_level, severity_tag>("Severity") << ","
        << "\""
        << expr::csv_decor[expr::stream << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")]
        << "\""
        << ",,"
        << "\"" << expr::csv_decor[expr::stream << expr::smessage] << "\"");
  } else {
    sink->set_formatter(expr::stream
                        << app_name << "," << app_version << ","
                        << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                        << ","
                        << expr::attr<severity_level, severity_tag>("Severity")
                        << ","
                        << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")
                        << ",," << expr::smessage);
  }

  sink->set_filter(expr::attr<std::string>("Channel") == channel_name);

  boost::log::core::get()->add_global_attribute(
      "Scopes", boost::log::attributes::named_scope());
  boost::log::core::get()->add_sink(sink);
  return sink;
}

inline boost::shared_ptr<cichannellog_date_hour_async_sink>
init_async_date_hour_channel_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name, const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool with_func_scope = false, bool csv_format = true,
    bool auto_flush = true) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_date_hour_based_backup_backend>(
      boost::filesystem::path(target), app_name, file_name_prefix_format,
      auto_flush);

  auto sink = boost::make_shared<cichannellog_date_hour_async_sink>(backend);

  std::string fs = "%F::%C:%l";
  if (with_func_scope) fs = "%F::%c:%l";
  if (csv_format) {
    sink->set_formatter(
        expr::stream
        << app_name << "," << app_version << ","
        << expr::format_date_time<boost::posix_time::ptime>(
               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
        << "," << expr::attr<severity_level, severity_tag>("Severity") << ","
        << "\""
        << expr::csv_decor[expr::stream << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")]
        << "\""
        << ",,"
        << "\"" << expr::csv_decor[expr::stream << expr::smessage] << "\"");
  } else {
    sink->set_formatter(expr::stream
                        << app_name << "," << app_version << ","
                        << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                        << ","
                        << expr::attr<severity_level, severity_tag>("Severity")
                        << ","
                        << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")
                        << ",," << expr::smessage);
  }

  sink->set_filter(expr::attr<std::string>("Channel") == channel_name);

  boost::log::core::get()->add_global_attribute(
      "Scopes", boost::log::attributes::named_scope());
  boost::log::core::get()->add_sink(sink);
  return sink;
}

inline boost::shared_ptr<cichannellog_async_sink>
init_async_channel_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    int64_t rotation_size = 100 * 100 * 1024, bool with_func_scope = false,
    bool csv_format = true, bool auto_flush = true) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_size_based_backup_backend>(
      boost::filesystem::path(target), file_name_suffix, rotation_size,
      auto_flush);

  auto sink = boost::make_shared<cichannellog_async_sink>(backend);

  std::string fs = "%F::%C:%l";
  if (with_func_scope) fs = "%F::%c:%l";
  if (csv_format) {
    sink->set_formatter(
        expr::stream
        << app_name << "," << app_version << ","
        << expr::format_date_time<boost::posix_time::ptime>(
               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
        << "," << expr::attr<severity_level, severity_tag>("Severity") << ","
        << "\""
        << expr::csv_decor[expr::stream << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")]
        << "\""
        << ",,"
        << "\"" << expr::csv_decor[expr::stream << expr::smessage] << "\"");
  } else {
    sink->set_formatter(expr::stream
                        << app_name << "," << app_version << ","
                        << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                        << ","
                        << expr::attr<severity_level, severity_tag>("Severity")
                        << ","
                        << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")
                        << ",," << expr::smessage);
  }

  sink->set_filter(expr::attr<std::string>("Channel") == channel_name &&
                   boost::phoenix::bind(&func_severity_filter,
                                        expr::attr<severity_level>("Severity"),
                                        severity_levels));

  boost::log::core::get()->add_global_attribute(
      "Scopes", boost::log::attributes::named_scope());
  boost::log::core::get()->add_sink(sink);
  return sink;
}

inline boost::shared_ptr<cichannellog_date_hour_async_sink>
init_async_date_hour_channel_level_logger(
    const std::string& app_name, const std::string& app_version,
    const std::string& channel_name,
    const std::vector<severity_level> severity_levels,
    const std::string& file_name_suffix, const std::string& target = "./log",
    const std::string& file_name_prefix_format = "%Y-%m-%d[%H]",
    bool with_func_scope = false, bool csv_format = true,
    bool auto_flush = true) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_date_hour_based_backup_backend>(
      boost::filesystem::path(target), file_name_suffix,
      file_name_prefix_format, auto_flush);

  auto sink = boost::make_shared<cichannellog_date_hour_async_sink>(backend);

  std::string fs = "%F::%C:%l";
  if (with_func_scope) fs = "%F::%c:%l";
  if (csv_format) {
    sink->set_formatter(
        expr::stream
        << app_name << "," << app_version << ","
        << expr::format_date_time<boost::posix_time::ptime>(
               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
        << "," << expr::attr<severity_level, severity_tag>("Severity") << ","
        << "\""
        << expr::csv_decor[expr::stream << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")]
        << "\""
        << ",,"
        << "\"" << expr::csv_decor[expr::stream << expr::smessage] << "\"");
  } else {
    sink->set_formatter(expr::stream
                        << app_name << "," << app_version << ","
                        << expr::format_date_time<boost::posix_time::ptime>(
                               "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                        << ","
                        << expr::attr<severity_level, severity_tag>("Severity")
                        << ","
                        << expr::format_named_scope(
                               "Scopes", boost::log::keywords::format = fs,
                               boost::log::keywords::depth = 1,
                               boost::log::keywords::incomplete_marker = "")
                        << ",," << expr::smessage);
  }

  sink->set_filter(expr::attr<std::string>("Channel") == channel_name &&
                   boost::phoenix::bind(&func_severity_filter,
                                        expr::attr<severity_level>("Severity"),
                                        severity_levels));

  boost::log::core::get()->add_global_attribute(
      "Scopes", boost::log::attributes::named_scope());
  boost::log::core::get()->add_sink(sink);
  return sink;
}

}  // namespace logger
}  // namespace castis
