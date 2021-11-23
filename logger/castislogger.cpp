#include "logger/castislogger.h"

#include <boost/algorithm/string.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/phoenix/bind/bind_function.hpp>
#include "fmt/chrono.h"

namespace fs = std::filesystem;

// The operator is used when putting the severity level to log
boost::log::formatting_ostream& operator<<(
    boost::log::formatting_ostream& strm,
    boost::log::to_log_manip<severity_level, severity_tag> const& manip) {
  static const char* strings[] = {
      "Foo",     "Debug", "Report", "Information", "Success",
      "Warning", "Error", "Fail",   "Exception",   "Critical"};

  severity_level level = manip.get();
  if (static_cast<std::size_t>(level) < sizeof(strings) / sizeof(*strings))
    strm << strings[level];
  else
    strm << static_cast<int>(level);

  return strm;
}

cilog_date_hour_backend::cilog_date_hour_backend(
    std::filesystem::path const& target_path, std::string_view file_name_suffix,
    std::string_view file_name_prefix_format, bool auto_flush)
    : auto_flush_(auto_flush),
      target_path_(target_path),
      file_name_suffix_(file_name_suffix),
      file_name_prefix_format_(file_name_prefix_format),
      current_date_hour_(get_current_date_hour()) {
  if (boost::starts_with(file_name_prefix_format_, "{:") == false ||
      boost::ends_with(file_name_prefix_format_, "}") == false) {
    file_name_prefix_format_ = "{:" + file_name_prefix_format_ + "}";
  }
}

void cilog_date_hour_backend::consume(boost::log::record_view const& /*rec*/,
                                      string_type const& formatted_message) {
  if (current_date_hour_ != get_current_date_hour()) {
    rotate_file();
  }

  if (!file_.is_open()) {
    file_path_ = generate_filepath();
    fs::create_directories(file_path_.parent_path());
    file_.open(file_path_, std::ofstream::out | std::ofstream::app);
    if (!file_.is_open()) {
      // failed to open file
      return;
    }

    auto link_filepath =
        file_path_.parent_path().parent_path() / (file_name_suffix_ + ".log");
    std::error_code ec;
    fs::remove(link_filepath, ec);
    if (!ec) {
      fs::create_symlink(fs::absolute(file_path_), link_filepath, ec);
    }
  }
  file_.write(formatted_message.data(),
              static_cast<std::streamsize>(formatted_message.size()));
  file_.put('\n');

  if (auto_flush_) file_.flush();

  if ((file_.is_open() && (current_date_hour_ != get_current_date_hour())) ||
      (!file_.good())) {
    rotate_file();
  }
}

void cilog_date_hour_backend::rotate_file() {
  file_.close();
  file_.clear();
  current_date_hour_ = get_current_date_hour();
}

std::filesystem::path cilog_date_hour_backend::generate_filepath() {
  if (file_name_prefix_format_.empty())
    file_name_prefix_format_ = "{:%Y-%m-%d[%H]}";
  auto filename_prefix = datetime_string_with_format(file_name_prefix_format_);
  auto monthly_path_name = datetime_string_with_format("{:%Y-%m}");

  auto monthly_path = target_path_ / monthly_path_name;

  // e.g. 2014-08-12[1]_example.log
  auto filename = fmt::format("{}_{}.log", filename_prefix, file_name_suffix_);
  return monthly_path / filename;
}

std::string cilog_date_hour_backend::datetime_string_with_format(
    std::string_view format) {
  auto now = std::time(nullptr);
  tm tm;
  boost::date_time::c_time::localtime(&now, &tm);
  return fmt::format(format, tm);
}

std::string cilog_date_hour_backend::get_current_date_hour() {
  return datetime_string_with_format("{:%Y-%m-%d %H}");
}

////////////////////////////////////////////////////////////////////////////////

cilog_backend::cilog_backend(std::filesystem::path const& target_path,
                             std::string_view file_name_suffix,
                             uintmax_t rotation_size, bool auto_flush)
    : auto_flush_(auto_flush),
      target_path_(target_path),
      file_name_suffix_(file_name_suffix),
      rotation_size_(rotation_size),
      characters_written_(0),
      current_date_(boost::gregorian::day_clock::local_day()) {}

void cilog_backend::consume(boost::log::record_view const& /*rec*/,
                            string_type const& formatted_message) {
  if (current_date_ != boost::gregorian::day_clock::local_day()) {
    rotate_file();
  }

  if (!file_.is_open()) {
    file_path_ = generate_filepath();
    fs::create_directories(file_path_.parent_path());
    file_.open(file_path_, std::ofstream::out | std::ofstream::app);
    if (!file_.is_open()) {
      // failed to open file
      return;
    }
    characters_written_ = static_cast<std::streamoff>(file_.tellp());

    auto link_filepath =
        file_path_.parent_path().parent_path() / (file_name_suffix_ + ".log");
    std::error_code ec;
    fs::remove(link_filepath, ec);
    if (!ec) {
      fs::create_symlink(fs::absolute(file_path_), link_filepath, ec);
    }
  }
  file_.write(formatted_message.data(),
              static_cast<std::streamsize>(formatted_message.size()));
  file_.put('\n');
  characters_written_ += formatted_message.size() + 1;

  if (auto_flush_) file_.flush();

  if ((file_.is_open() && (characters_written_ >= rotation_size_)) ||
      (!file_.good())) {
    rotate_file();
  }
}

void cilog_backend::rotate_file() {
  file_.close();
  file_.clear();
  characters_written_ = 0;
  current_date_ = boost::gregorian::day_clock::local_day();
}

std::filesystem::path cilog_backend::generate_filepath() {
  auto filename_prefix = datetime_string_with_format("{:%Y-%m-%d}");
  auto monthly_path_name = datetime_string_with_format("{:%Y-%m}");

  auto monthly_path = target_path_ / monthly_path_name;
  // e.g. 2014-08-12[1]_example.log
  std::regex pattern(filename_prefix + "(\\[[0-9]+\\])?" + "_" +
                     file_name_suffix_ + ".log");
  auto next_index = scan_next_index(monthly_path, pattern);

  auto filename =
      (next_index > 0)
          ? fmt::format("{}[{}]_{}.log", filename_prefix, next_index,
                        file_name_suffix_)
          : fmt::format("{}_{}.log", filename_prefix, file_name_suffix_);

  return monthly_path / filename;
}

std::string cilog_backend::datetime_string_with_format(
    std::string_view format) {
  auto now = std::time(nullptr);
  tm tm;
  boost::date_time::c_time::localtime(&now, &tm);
  return fmt::format(format, tm);
}

uintmax_t cilog_backend::scan_next_index(std::filesystem::path const& path,
                                         std::regex const& pattern) {
  uintmax_t current_index = 0;
  fs::path current_fs;
  if (fs::exists(path) && fs::is_directory(path)) {
    for (const auto& entry : fs::directory_iterator(path)) {
      const auto& p = entry.path();
      auto filename = p.filename().string();
      if (std::regex_match(filename, pattern)) {
        uintmax_t index = parse_index(filename);
        if (index >= current_index) {
          current_index = index;
          current_fs = p;
        }
      }
    }
  }
  std::error_code ec;
  auto filesize = fs::file_size(current_fs, ec);
  if (!ec) {
    if (filesize >= rotation_size_) {
      ++current_index;
    }
  }
  return current_index;
}

uintmax_t cilog_backend::parse_index(std::string const& filename) {
  auto pos_index_begin = filename.find('[');
  auto pos_index_end = filename.find(']');
  unsigned int index = 0;
  if (pos_index_begin != std::string::npos &&
      pos_index_end != std::string::npos) {
    index = atoi(
        filename.substr(pos_index_begin + 1, pos_index_end - pos_index_begin)
            .c_str());
  }
  return index;
}

////////////////////////////////////////////////////////////////////////////////

namespace castis {
namespace logger {
void init_logger(std::string app_name, std::string app_version,
                 std::string_view target /* = "./log"*/,
                 int64_t rotation_size /* = 10 * 1024 * 1024*/,
                 bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_backend>(fs::path(target), app_name,
                                                   rotation_size, auto_flush);

  auto sink = boost::make_shared<cilog_sync_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(expr::attr<std::string>("Channel") ==
                   CASTIS_CILOG_DEFAULT_MODULUE);

  boost::log::core::get()->add_sink(sink);
}

boost::shared_ptr<cilog_async_sink_t> init_async_logger(
    std::string app_name, std::string app_version,
    std::string_view target /* = "./log"*/,
    int64_t rotation_size /* = 10 * 1024 * 1024*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_backend>(fs::path(target), app_name,
                                                   rotation_size, auto_flush);

  auto sink = boost::make_shared<cilog_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(expr::attr<std::string>("Channel") ==
                   CASTIS_CILOG_DEFAULT_MODULUE);

  boost::log::core::get()->add_sink(sink);

  return sink;
}

bool func_module_severity_filter(
    boost::log::value_ref<std::string> const& ch,
    boost::log::value_ref<severity_level> const& level,
    const std::vector<Module>& modules) {
  for (const auto& m : modules) {
    if (m.name_ == ch || (m.name_.empty() && ch != "access")) {
      if (m.level_type_ == Module::min_level) {
        return level >= m.min_level_;
      } else {
        return m.specific_levels_.find(level.get()) != m.specific_levels_.end();
      }
    }
  }
  return false;
}

boost::shared_ptr<cilog_async_sink_t> init_async_module_logger(
    std::string app_name, std::string app_version,
    const std::vector<Module>& filters, std::string_view file_name_suffix,
    std::string_view target /* = "./log"*/,
    int64_t rotation_size /* = 10 * 1024 * 1024*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();

  std::vector<boost::shared_ptr<cilog_async_sink_t>> sinks;
  auto backend = boost::make_shared<cilog_backend>(
      fs::path(target), file_name_suffix, rotation_size, auto_flush);

  auto sink = boost::make_shared<cilog_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(boost::phoenix::bind(
      &func_module_severity_filter, expr::attr<std::string>("Channel"),
      expr::attr<severity_level>("Severity"), filters));

  boost::log::core::get()->add_sink(sink);
  sinks.push_back(sink);

  return sink;
}

bool func_module_ptr_severity_filter(
    boost::log::value_ref<std::string> const& ch,
    boost::log::value_ref<severity_level> const& level,
    const std::vector<std::shared_ptr<Module>>& modules) {
  for (const auto& m : modules) {
    if (m->name_ == ch || (m->name_.empty() && ch != "access")) {
      if (m->level_type_ == Module::min_level) {
        return level >= m->min_level_;
      } else {
        return m->specific_levels_.find(level.get()) !=
               m->specific_levels_.end();
      }
    }
  }
  return false;
}

boost::shared_ptr<cilog_async_sink_t> init_async_module_logger(
    std::string app_name, std::string app_version,
    const std::vector<std::shared_ptr<Module>>& filters,
    std::string_view file_name_suffix, std::string_view target /* = "./log"*/,
    int64_t rotation_size /* = 10 * 1024 * 1024*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();

  std::vector<boost::shared_ptr<cilog_async_sink_t>> sinks;
  auto backend = boost::make_shared<cilog_backend>(
      fs::path(target), file_name_suffix, rotation_size, auto_flush);

  auto sink = boost::make_shared<cilog_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(boost::phoenix::bind(
      &func_module_ptr_severity_filter, expr::attr<std::string>("Channel"),
      expr::attr<severity_level>("Severity"), filters));

  boost::log::core::get()->add_sink(sink);
  sinks.push_back(sink);

  return sink;
}

boost::shared_ptr<cilog_date_hour_async_sink_t> init_async_date_hour_logger(
    std::string app_name, std::string app_version,
    std::string_view target /* = "./log"*/,
    std::string_view file_name_prefix_format /* = "%Y-%m-%d[%H]"*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_date_hour_backend>(
      fs::path(target), app_name, file_name_prefix_format, auto_flush);

  auto sink = boost::make_shared<cilog_date_hour_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(expr::attr<std::string>("Channel") ==
                   CASTIS_CILOG_DEFAULT_MODULUE);

  boost::log::core::get()->add_sink(sink);

  return sink;
}

bool func_severity_filter(boost::log::value_ref<severity_level> const& level,
                          const std::vector<severity_level>& severity_levels) {
  for (auto seveirty_level : severity_levels) {
    if (level == seveirty_level) {
      return true;
    }
  }
  return false;
}

boost::shared_ptr<cilog_async_sink_t> init_async_level_logger(
    std::string app_name, std::string app_version,
    const std::vector<severity_level> severity_levels,
    std::string_view file_name_suffix, std::string_view target /* = "./log"*/,
    int64_t rotation_size /* = 10 * 1024 * 1024*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_backend>(
      fs::path(target), file_name_suffix, rotation_size, auto_flush);

  auto sink = boost::make_shared<cilog_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(expr::attr<std::string>("Channel") ==
                       CASTIS_CILOG_DEFAULT_MODULUE &&
                   boost::phoenix::bind(&func_severity_filter,
                                        expr::attr<severity_level>("Severity"),
                                        severity_levels));

  boost::log::core::get()->add_sink(sink);

  return sink;
}

boost::shared_ptr<cilog_date_hour_async_sink_t>
init_async_date_hour_level_logger(
    std::string app_name, std::string app_version,
    const std::vector<severity_level> severity_levels,
    std::string_view file_name_suffix, std::string_view target /* = "./log"*/,
    std::string_view file_name_prefix_format /* = "%Y-%m-%d[%H]"*/,
    bool auto_flush /* = true*/) {
  namespace expr = boost::log::expressions;
  boost::log::add_common_attributes();
  auto backend = boost::make_shared<cilog_date_hour_backend>(
      fs::path(target), file_name_suffix, file_name_prefix_format, auto_flush);

  auto sink = boost::make_shared<cilog_date_hour_async_sink_t>(backend);
  sink->set_formatter(expr::stream
                      << app_name << "," << app_version << ","
                      << expr::format_date_time<boost::posix_time::ptime>(
                             "TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
                      << ","
                      << expr::attr<severity_level, severity_tag>("Severity")
                      << "," << expr::smessage);

  sink->set_filter(expr::attr<std::string>("Channel") ==
                       CASTIS_CILOG_DEFAULT_MODULUE &&
                   boost::phoenix::bind(&func_severity_filter,
                                        expr::attr<severity_level>("Severity"),
                                        severity_levels));

  boost::log::core::get()->add_sink(sink);

  return sink;
}

}  // namespace logger
}  // namespace castis
