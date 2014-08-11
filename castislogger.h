#include <cstddef>
#include <iostream>
#include <boost/regex.hpp>
#include <boost/bind.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/utility/formatting_ostream.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/support/date_time.hpp>
#include <boost/log/sources/global_logger_storage.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/filesystem/operations.hpp>

namespace logging = boost::log;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace sinks = boost::log::sinks;

#define CILOG(severity)\
  BOOST_LOG_SEV(my_logger::get(), severity) << boost::filesystem::path(__FILE__).filename().string() << "::" << __FUNCTION__ << ":" << __LINE__ << ",,"

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

std::ostream& operator<< (std::ostream& strm, severity_level level)
{
  static const char* strings[] =
  {
    "Foo",
    "Debug",
    "Report",
    "Information",
    "Success",
    "Warning",
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

struct severity_tag;
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
    "Warning",
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

class cilog_backend:
  public sinks::basic_formatted_sink_backend<char, sinks::synchronized_feeding>
{
  private:
    bool auto_flush_;
    boost::filesystem::ofstream file_;
    boost::filesystem::path target_path_;
    boost::filesystem::path file_path_;
    std::string file_name_suffix_;
    uintmax_t rotation_size_;
    uintmax_t characters_written_;

  public:
    explicit cilog_backend(
        boost::filesystem::path const& target_path,
        std::string const& file_name_suffix,
        uintmax_t rotation_size,
        bool auto_flush)
      : auto_flush_(auto_flush),
        target_path_(target_path),
        file_name_suffix_(file_name_suffix),
        rotation_size_(rotation_size),
        characters_written_(0)
    {
    }
    void consume(logging::record_view const& rec, string_type const& formatted_message)
    {
      if (
          (file_.is_open() &&
           (characters_written_ + formatted_message.size() >= rotation_size_)
           ) ||
        !file_.good()
        )
      {
        rotate_file();
      }
      if (!file_.is_open())
      {
        file_path_ = generate_filepath();
        boost::filesystem::create_directories(file_path_.parent_path());
        file_.open(file_path_);
        if (!file_.is_open())
        {
          return;
        }
        characters_written_ = static_cast<std::streamoff>(file_.tellp());
      }
      file_.write(formatted_message.data(), static_cast<std::streamsize>(formatted_message.size()));
      file_.put('\n');
      characters_written_ += formatted_message.size() + 1;
      if (auto_flush())
      {
        file_.flush();
      }
    }
    void rotate_file()
    {
      file_.close();
      file_.clear();
      characters_written_ = 0;
    }
    bool auto_flush()
    {
      return auto_flush_;
    }
    boost::filesystem::path generate_filepath()
    {
      std::locale locale_ymd(std::cout.getloc(), new boost::posix_time::time_facet("%Y-%m-%d"));
      std::stringstream filename_prefix_ss;
      filename_prefix_ss.imbue(locale_ymd);
      filename_prefix_ss << boost::posix_time::second_clock::universal_time();;

      std::locale locale_ym(std::cout.getloc(), new boost::posix_time::time_facet("%Y-%m"));
      std::stringstream monthly_path_ss;
      monthly_path_ss.imbue(locale_ym);
      monthly_path_ss << boost::posix_time::second_clock::universal_time();;
      boost::filesystem::path monthly_target_path(target_path_ / monthly_path_ss.str());

      uintmax_t file_index = 0;
      boost::regex pattern(filename_prefix_ss.str() + "[\\[\\]0-9]*" + "_" + file_name_suffix_ + ".log");
      if (boost::filesystem::exists(monthly_target_path) && boost::filesystem::is_directory(monthly_target_path))
      {
        boost::filesystem::directory_iterator it(monthly_target_path), end;
        for (; it != end; ++it)
        {
          if (boost::regex_match(it->path().filename().string(), pattern))
          {
            std::string filename = it->path().filename().string();
            size_t pos_index_begin = filename.find('[');
            size_t pos_index_end = filename.find(']');
            unsigned int index = 0;
            if (pos_index_begin != std::string::npos && pos_index_end != std::string::npos)
            {
              index = atoi(filename.substr(pos_index_begin + 1, pos_index_end - pos_index_begin).c_str());
            }
            if (index >= file_index)
            {
              file_index = index + 1;
            }
          }
        }
      }

      std::stringstream filename_ss;
      filename_ss << filename_prefix_ss.str();
      if (file_index > 0)
      {
        filename_ss << "[" << file_index << "]";
      }
      filename_ss << "_" + file_name_suffix_;
      filename_ss << ".log";
      return boost::filesystem::path(monthly_target_path / filename_ss.str());
    }
};

namespace castis
{
  namespace logger
  {
    static void init(
        const std::string& app_name,
        const std::string& app_version,
        const std::string& target,
        unsigned long long rotation_size = 100 * 100 * 1024,
        bool auto_flush = true
        )
    {
      logging::add_common_attributes();
      typedef sinks::synchronous_sink< cilog_backend > sink_t;
      boost::shared_ptr<sink_t> sink(new sink_t(boost::filesystem::path(target), app_name, rotation_size, auto_flush));
      sink->set_formatter(
          expr::stream
          << app_name
          << "," << app_version
          << "," << expr::format_date_time<boost::posix_time::ptime>("TimeStamp", "%Y-%m-%d,%H:%M:%S.%f")
          << "," << expr::attr<severity_level, severity_tag>("Severity")
          << "," << expr::smessage
          );
      logging::core::get()->add_sink(sink);
    }
  }
}

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::severity_logger_mt<severity_level>)

