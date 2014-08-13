# castislogger

castis-formatted logger, based on [Boost.log](http://www.boost.org/doc/libs/1_55_0b1/libs/log/doc/html/index.html)

## Features

* severity levels
* file rotation (based on size)
* auto flushing
* support both ostream and printf-style format

## Example

```cpp
#include "castislogger.h"

int main()
{
  castis::logger::init("example", "1.0.0", "./log");

  // support severity levels
  CILOG(debug) << "A debug message";
  CILOG(error) << "An error message";

  // support both streams and printf-style format
  CILOG(report) << "strings(" << "abc" << "), integers(" << 123 << ")";
  CILOGF(report, "strings(%s), integers(%d)", "abc", 123);

  return 0;
}
```

will create log file at `./log/2014-08/2014-08-13_example.log` and record,

```
example,1.0.0,2014-08-13,19:10:28.872906,Debug,example.cpp::main:8,,A debug message
example,1.0.0,2014-08-13,19:10:28.873859,Error,example.cpp::main:9,,An error message
example,1.0.0,2014-08-13,19:10:28.873894,Report,example.cpp::main:12,,strings(abc), integers(123)
example,1.0.0,2014-08-13,19:10:28.873920,Report,example.cpp::main:13,,strings(abc), integers(123)
```

## Performance

About `5 us` latency per one line of log.
Logged 1,000,000 lines, measured with [profc](https://bitbucket.org/teamd7/profc)

```
--------------------------------------------------------------
name                           count      elapsed      us/call
logging-format               1000000       6506ms          6us
logging                      1000000       5234ms          5us
```

## Dependency

* Boost 1.56
    * log
    * thread
    * filesystem
    * system
    * regex
    * format

