#include "castislogger.h"

int main()
{
  castis::logger::init("example", "1.0.0", "./log");

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

  for (int i = 0; i < 100000; ++i)
  {
    CILOG(info) << i << "th log" << " with " << "some message";
  }

  return 0;
}

