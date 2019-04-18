#include "logger/castislogger.h"

BOOST_LOG_ATTRIBUTE_KEYWORD(severity, "Severity", severity_level)

int main() {
  auto sink = castis::logger::init_async_logger("example", "1.0.0");
  boost::log::core::get()->set_filter(severity >= warning);

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
  CILOG(report, "strings(%s), integers(%d), float numbers(%.2f)...",
                 "abc", 1, 3.14);

  for (int i = 0; i < 1000000; ++i) {
    CILOG(info) << i << "th log with some message";
  }

  castis::logger::stop_logger(sink);

  return 0;
}

