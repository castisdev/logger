#include <chrono>
#include <thread>
#include "logger/castisaccesslogger.h"

int main() {
  std::vector<boost::shared_ptr<castis::logger::cilog_async_sink_t>> sinks;
  std::vector<castis::logger::Module> f1;
  f1.push_back(castis::logger::Module("module1", debug));
  f1.push_back(castis::logger::Module("access", debug));
  auto sink1 = castis::logger::init_async_module_logger("module1", "1.0.0", f1,
                                                        "module1");
  sinks.push_back(sink1);

  using HttpAccess = castis::logger::HttpAccess;
  auto sinkaccess = castis::logger::init_httpaccess_logger("access");
  sinks.push_back(sinkaccess);

  auto request1_time = boost::posix_time::microsec_clock::universal_time();
  HttpAccess a = HttpAccess{
      "142.43.55.13",
      "main",
      "main",
      castis::logger::httpaccesslog::request_time(request1_time),
      castis::logger::httpaccesslog::request_line("GET", "/foo", 1, 1),
      200,
      1,
      "http://www.google.com/",
      "chrome/10.0",
      100};
  ACCESSLOG(a);

  auto request2_time = boost::posix_time::microsec_clock::universal_time();
  std::this_thread::sleep_for(std::chrono::seconds(2));

  HttpAccess b = HttpAccess{
      "142.43.55.13",
      "",
      "",
      castis::logger::httpaccesslog::request_time(request2_time),
      castis::logger::httpaccesslog::request_line("GET", "/longtime", 1, 1),
      200,
      5016,
      "http://www.google.com/",
      "chrome/10.0",
      castis::logger::httpaccesslog::serve_duration(request2_time)};

  ACCESSLOG(b);
  CIMLOG(module1, debug) << "Just a foo";

  for (auto& sink : sinks) {
    castis::logger::stop_logger(sink);
  }

  return 0;
}
