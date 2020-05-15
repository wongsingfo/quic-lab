#include "stopwatch.h"
#include "errno.h"
#include "string_raw.h"

static Stopwatch global_;

Stopwatch::Stopwatch() {
  gettimeofday(&tv_, NULL);
}

uint64_t Stopwatch::getElapsedInMicroseconds() {
  struct timeval now;
  if (gettimeofday(&now, NULL) < 0) {
    throw std::runtime_error("gettimeofday failed");
    exit(1);
  }

  return subtractTimeval(&now, &tv_);
}

uint64_t Stopwatch::getGlobalTimeInMicroseconds() {
  return global_.getElapsedInMicroseconds();
}

uint64_t 
Stopwatch::subtractTimeval(struct timeval *a, struct timeval *b) {
  struct timeval elapsed;
  timersub(a, b, &elapsed);

  return elapsed.tv_sec * 1000000 + elapsed.tv_usec;
}
