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

  using Access = castis::logger::Access;
  auto sinkaccess = castis::logger::init_access_logger("access");
  sinks.push_back(sinkaccess);

  auto request1_time = boost::posix_time::microsec_clock::universal_time();
  Access a = Access{
      "142.43.55.13",
      "main",
      "main",
      castis::logger::accesslog::request_time(request1_time),
      castis::logger::accesslog::request_line("GET", "/foo", 1, 1),
      200,
      1,
      "http://www.google.com/",
      "chrome/10.0",
      100};
  ACCESSLOG(a);

  auto request2_time = boost::posix_time::microsec_clock::universal_time();
  std::this_thread::sleep_for(std::chrono::seconds(2));

  Access b = Access{
      "142.43.55.13",
      "",
      "",
      castis::logger::accesslog::request_time(request2_time),
      castis::logger::accesslog::request_line("GET", "/longtime", 1, 1),
      200,
      5016,
      "http://www.google.com/",
      "chrome/10.0",
      castis::logger::accesslog::serve_duration(request2_time)};

  ACCESSLOG(b);
  CIMLOG(module1, debug) << "Just a foo";

  for (auto& sink : sinks) {
    castis::logger::stop_logger(sink);
  }

  return 0;
}
