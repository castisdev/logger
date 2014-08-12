# castislogger

castis-formatted logger, based on [Boost.log](http://www.boost.org/doc/libs/1_55_0b1/libs/log/doc/html/index.html)

## Features

* severity level filtering
* file rotation (size based)
* auto flushing


## Example

```cpp
#include "castislogger.h"
int main()
{
  castis::logger::init("example", "1.0.0", "./log");
  CILOG(debug) << "A debug message";
  CILOG(error) << "An error severity message";
  return 0;
}
```

generates log file `./log/2014-08/2014-08-07_example.log` and record,

```
example,1.0.0,2014-08-07,16:45:12.862521,Debug,example.cpp::main:8,,A debug message
example,1.0.0,2014-08-07,16:45:12.862916,Error,example.cpp::main:9,,An error severity message
```

## Dependency

* Boost 1.56
    * log
    * thread
    * filesystem
    * system
    * regex

