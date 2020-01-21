#include "logger/castislogger.h"

int main() {
  std::vector<castis::logger::Module> f1;
  f1.push_back(castis::logger::Module("module1", debug));
  auto sink1 = castis::logger::init_async_module_logger("example1", "1.0.0", f1, "example1");

  std::vector<castis::logger::Module> f2;
  f2.push_back(castis::logger::Module("default", warning));
  f2.push_back(castis::logger::Module("module2", info));
  auto sink2 = castis::logger::init_async_module_logger("example2", "1.0.0", f2, "example2");

  std::vector<castis::logger::Module> f3;
  f3.push_back(castis::logger::Module("module1", std::set<severity_level>{foo, error}));
  f3.push_back(castis::logger::Module("module3", std::set<severity_level>{warning}));
  auto sink3 = castis::logger::init_async_module_logger("example3", "1.0.0", f3, "example3");

  std::vector<std::shared_ptr<castis::logger::Module>> f4;
  f4.push_back(std::make_shared<castis::logger::Module>(exception));
  auto sink4 = castis::logger::init_async_module_logger("example4", "1.0.0", f4, "example4");

  std::vector<std::shared_ptr<castis::logger::Module>> f5;
  f5.push_back(std::make_shared<castis::logger::Module>(std::set<severity_level>{info}));
  auto sink5 = castis::logger::init_async_module_logger("example5", "1.0.0", f5, "example5");

  CILOG(foo) << "Just a foo";
  CILOG(debug) << "A normal severity message";
  CILOG(report) << "A notification severity message";
  CILOG(warning) << "A warning severity message";
  CILOG(info) << "A information severity message";
  CILOG(error) << "A error severity message";
  CILOG(fail) << "A fail severity message";
  CILOG(success) << "A success severity message";
  CILOG(exception) << "A exception severity message";
  CILOG(critical) << "A critical severity message";
  CILOG(critical, "format");
  CILOG(critical, "format, %d", 1234);
  CILOG(critical, "format, %d, %s", 1234, "string");

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
  CIMLOG(module1, critical, "format");
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
