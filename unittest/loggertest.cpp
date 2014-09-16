#include <fstream>
#include <gtest/gtest.h>
#include "castislogger.h"

std::string datetime_string_with_format(std::string const& format) {
  std::locale loc(
      std::cout.getloc(),
      new boost::posix_time::time_facet(format.c_str()));
  std::stringstream ss;
  ss.imbue(loc);
  ss << boost::posix_time::second_clock::universal_time();;
  return ss.str();
}

TEST(LoggerTest, dummy) {
  castis::logger::init_logger("example", "1.0.0");
  
  // add a log line
  CILOG(foo) << "Just a foo";

  // Check log file existance
  std::string filepath = datetime_string_with_format("./log/%Y-%m/%Y-%m-%d_example.log");
  std::ifstream file(filepath);
  ASSERT_TRUE(file.is_open());
}
