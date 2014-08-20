# Castis Logger

[Boost.log](http://www.boost.org/doc/libs/1_55_0b1/libs/log/doc/html/index.html)를 이용하면서 기존 CiLogger와 동일한 포멧으로 log를 남길 수 있도록 도와주는 header-only 라이브러리

## Features

* severity levels
* file rotation (size 기반, default = 10MB)
* auto flushing on/off
* iostream 과 printf 스타일을 모두 사용 가능
* asyncronous logging

## Basic Example

```cpp
#include "castislogger.h"

int main()
{
  castis::logger::init_logger("example", "1.0.0");

  // support severity levels
  CILOG(debug) << "A debug message";
  CILOG(error) << "An error message";

  // support both streams and printf-style format
  CILOG(report) << "strings(" << "abc" << "), integers(" << 123 << ")";
  CILOGF(report, "strings(%s), integers(%d)", "abc", 123);

  return 0;
}
```

이 예제는 `./log/2014-08/2014-08-13_example.log` 경로의 파일을 생성하여 아래와 같은 log를 기록합니다.

```
example,1.0.0,2014-08-13,19:10:28.872906,Debug,example.cpp::main:8,,A debug message
example,1.0.0,2014-08-13,19:10:28.873859,Error,example.cpp::main:9,,An error message
example,1.0.0,2014-08-13,19:10:28.873894,Report,example.cpp::main:12,,strings(abc), integers(123)
example,1.0.0,2014-08-13,19:10:28.873920,Report,example.cpp::main:13,,strings(abc), integers(123)
```

## Asyncronous Logging Example

Asyncronous logging을 사용하면 I/O 동작을 별도의 thread에서 수행하므로 application의 성능을 향상시킬 수 있습니다.

```cpp
#include "castislogger.h"

int main()
{
  auto sink = castis::logger::init_async_logger("example", "1.0.0");

  CILOG(debug) << "A debug message";
  CILOG(error) << "An error message";

  castis::logger::stop_logger(sink);
  return 0;
}
```

## Performance

각각 1,000,000 라인의 로그를 남기는 성능 테스트 결과입니다.(auto-flush enabled)

latency 측정은 [profc](https://bitbucket.org/teamd7/profc)를 이용했습니다.

```
$ ./example && ./example_async
--------------------------------------------------------------
name                           count      elapsed      us/call
logging_printf_style         1000000      10291ms         10us
logging                      1000000       7260ms          7us

--------------------------------------------------------------
name                           count      elapsed      us/call
logging_async                1000000       3725ms          3us

```

## Dependency

* Boost 1.56
    * log
    * thread
    * filesystem
    * system
    * regex
    * format