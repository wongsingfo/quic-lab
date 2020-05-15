//
// Created by Chengke Wong on 2019/12/07.
//

#ifndef SRC_STOPWATCH
#define SRC_STOPWATCH

#include <cstdint>
#include <sys/time.h>
#include <stdexcept>
#include <cstdlib>
#include <string>

class Stopwatch {
 public:

  Stopwatch();

  uint64_t getElapsedInMicroseconds();

  static uint64_t getGlobalTimeInMicroseconds();

 private:
  struct timeval tv_;

  uint64_t subtractTimeval(struct timeval *a, struct timeval *b);
};


#endif //SRC_STOPWATCH
