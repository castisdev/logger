#include "logger/castislogger.h"

int main() {
  auto sink = castis::logger::init_async_logger("example", "1.0.0");
  auto sink_hour = castis::logger::init_async_date_hour_logger("example_hour", "1.0.0");
  auto sink_hour_level_sink = castis::logger::init_async_date_hour_level_logger(
    "example", "1.0.0", {error, exception}, "error_exception");
  
  // support severity levels
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

  // support both streams and printf-style format
  CILOG(report) << "strings(" << "abc" << "), integers(" << 1
                << "), float numbers(" << 3.14 << ")...";
  CILOGF(report, "strings(%s), integers(%d), float numbers(%.2f)...",
                 "abc", 1, 3.14);

  for (int i = 0; i < 1000000; ++i) {
    CILOG(info) << i << "th log with some message";
  }

  castis::logger::stop_logger(sink);
  castis::logger::stop_logger(sink_hour);
  castis::logger::stop_logger(sink_hour_level_sink);

  return 0;
}

