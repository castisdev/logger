#include "logger/castislogger.h"
#include "logger/cichannellogger.h"

struct Filter {
  enum { min_level, specific_level };

  Filter(const std::string &module, severity_level level)
      : module_(module), level_type_(Filter::min_level), min_level_(level) {}
  Filter(const std::string &module, std::set<severity_level> specific_levels)
      : module_(module),
        level_type_(Filter::specific_level),
        specific_levels_(specific_levels) {}
  Filter(severity_level level)
      : level_type_(Filter::min_level), min_level_(level) {}
  Filter(std::set<severity_level> specific_levels)
      : level_type_(Filter::specific_level),
        specific_levels_(specific_levels) {}

  std::string module_;
  int level_type_{min_level};
  severity_level min_level_{info};
  std::set<severity_level> specific_levels_;
};

typedef std::vector<Filter> filterList;
inline bool func_severity_filter(
    boost::log::value_ref<std::string> const &ch,
    boost::log::value_ref<severity_level> const &level,
    const filterList &filters) {
  for (const auto &f : filters) {
    if (f.module_.empty() || f.module_ == ch) {
      if (f.level_type_ == Filter::min_level) {
        return level >= f.min_level_;
      } else {
        return f.specific_levels_.find(level.get()) != f.specific_levels_.end();
      }
    }
  }
  return false;
}

namespace expr = boost::log::expressions;

int main() {
  auto sink1 = castis::logger::init_async_logger("example1", "1.0.0");
  auto sink2 = castis::logger::init_async_logger("example2", "1.0.0");
  auto sink3 = castis::logger::init_async_logger("example3", "1.0.0");
  auto sink4 = castis::logger::init_async_logger("example4", "1.0.0");
  auto sink5 = castis::logger::init_async_logger("example5", "1.0.0");

  {
    filterList filters;
    filters.push_back(Filter("module1", debug));
    sink1->reset_filter();
    sink1->set_filter(boost::phoenix::bind(
        &func_severity_filter, expr::attr<std::string>("Channel"),
        expr::attr<severity_level>("Severity"), filters));
  }

  {
    filterList filters;
    filters.push_back(Filter("default", info));
    filters.push_back(Filter("module2", info));
    sink2->reset_filter();
    sink2->set_filter(boost::phoenix::bind(
        &func_severity_filter, expr::attr<std::string>("Channel"),
        expr::attr<severity_level>("Severity"), filters));
  }

  {
    filterList filters;
    filters.push_back(Filter("module1", std::set<severity_level>{foo, error}));
    filters.push_back(Filter("module3", std::set<severity_level>{warning}));
    sink3->reset_filter();
    sink3->set_filter(boost::phoenix::bind(
        &func_severity_filter, expr::attr<std::string>("Channel"),
        expr::attr<severity_level>("Severity"), filters));
  }

  {
    filterList filters;
    filters.push_back(Filter(exception));
    sink4->reset_filter();
    sink4->set_filter(boost::phoenix::bind(
        &func_severity_filter, expr::attr<std::string>("Channel"),
        expr::attr<severity_level>("Severity"), filters));
  }

  {
    filterList filters;
    filters.push_back(Filter(std::set<severity_level>{info}));
    sink5->reset_filter();
    sink5->set_filter(boost::phoenix::bind(
        &func_severity_filter, expr::attr<std::string>("Channel"),
        expr::attr<severity_level>("Severity"), filters));
  }

  CIMLOG(module1, foo) << "Just a foo";
  CIMLOG(module1, debug) << "A normal severity message";
  CIMLOG(module1, report) << "A notification severity message";
  CIMLOG(module1, warning) << "A warning severity message";
  CIMLOG(module1, info) << "A information severity message";
  CIMLOG(module1, error) << "A error severity message";
  CIMLOG(module1, fail) << "A fail severity message";
  CIMLOG(module1, success) << "A success severity message";
  CIMLOG(module1, exception) << "A exception severity message";
  CIMLOG(module1, critical) << "A critical severity message";
  CIMLOG(module1, critical, "format, %d", 1234);
  CIMLOG(module1, critical, "format, %d, %s", 1234, "string");

  CIMLOG(module2, foo) << "Just a foo";
  CIMLOG(module2, debug) << "A normal severity message";
  CIMLOG(module2, report) << "A notification severity message";
  CIMLOG(module2, warning) << "A warning severity message";
  CIMLOG(module2, info) << "A information severity message";
  CIMLOG(module2, error) << "A error severity message";
  CIMLOG(module2, fail) << "A fail severity message";
  CIMLOG(module2, success) << "A success severity message";
  CIMLOG(module2, exception) << "A exception severity message";
  CIMLOG(module2, critical) << "A critical severity message";
  CIMLOG(module2, critical, "format, %d", 1234);
  CIMLOG(module2, critical, "format, %d, %s", 1234, "string");

  CIMLOG(module3, foo) << "Just a foo";
  CIMLOG(module3, debug) << "A normal severity message";
  CIMLOG(module3, report) << "A notification severity message";
  CIMLOG(module3, warning) << "A warning severity message";
  CIMLOG(module3, info) << "A information severity message";
  CIMLOG(module3, error) << "A error severity message";
  CIMLOG(module3, fail) << "A fail severity message";
  CIMLOG(module3, success) << "A success severity message";
  CIMLOG(module3, exception) << "A exception severity message";
  CIMLOG(module3, critical) << "A critical severity message";
  CIMLOG(module3, critical, "format, %d", 1234);
  CIMLOG(module3, critical, "format, %d, %s", 1234, "string");

  castis::logger::stop_logger(sink1);
  castis::logger::stop_logger(sink2);
  castis::logger::stop_logger(sink3);
  castis::logger::stop_logger(sink4);
  castis::logger::stop_logger(sink5);

  return 0;
}
