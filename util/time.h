//
// Created by Chengke Wong on 2019/9/28.
//
// Modified by Chengke Wong on 2020/04/30.
//

#ifndef SRC_BASE_TIME_BASE_H
#define SRC_BASE_TIME_BASE_H

#include <cstdint>
#include <limits>
#include <ostream>
#include <cmath>

class Instant;

// A Duration represents the signed difference between two points in
// time, stored in microsecond resolution.
class Duration {
public:
    // Create a object with an offset of 0.
    static constexpr Duration zero() { return Duration(0); }

    // Create a object with infinite offset time.
    static constexpr Duration infinite() { return Duration(kInfiniteTimeUs); }

    // Converts a number of seconds to a time offset.
    static constexpr Duration fromSeconds(int64_t secs) {
        return Duration(secs * 1000 * 1000);
    }

    // Converts a number of milliseconds to a time offset.
    static constexpr Duration fromMilliseconds(int64_t ms) {
        return Duration(ms * 1000);
    }

    // Converts a number of microseconds to a time offset.
    static constexpr Duration fromMicroseconds(int64_t us) { return Duration(us); }

    // Converts the time offset to a rounded number of seconds.
    inline int64_t toSeconds() const { return time_offset_ / 1000 / 1000; }

    // Converts the time offset to a rounded number of milliseconds.
    inline int64_t toMilliseconds() const { return time_offset_ / 1000; }

    // Converts the time offset to a rounded number of microseconds.
    inline int64_t toMicroseconds() const { return time_offset_; }

    inline struct timeval toTimeval() const {
        return timeval {
          .tv_sec = static_cast<time_t>(time_offset_ / 1000000),
          .tv_usec = static_cast<suseconds_t>(time_offset_ % 1000000) 
        };
    }

    inline bool isZero() const { return time_offset_ == 0; }

    inline bool isInfinite() const {
        return time_offset_ == kInfiniteTimeUs;
    }

    std::string toDebugValue() const;

    static const int64_t kInfiniteTimeUs =
        std::numeric_limits<int64_t>::max();

private:
    friend inline bool operator==(Duration lhs, Duration rhs);

    friend inline bool operator<(Duration lhs, Duration rhs);

    friend inline Duration operator<<(Duration lhs, size_t rhs);

    friend inline Duration operator>>(Duration lhs, size_t rhs);

    friend inline Duration operator+(Duration lhs,
                                              Duration rhs);

    friend inline Duration operator-(Duration lhs,
                                              Duration rhs);

    friend inline Duration operator*(Duration lhs, int rhs);

    friend inline Duration operator*(Duration lhs, double rhs);

    friend inline Instant operator+(Instant lhs, Duration rhs);

    friend inline Instant operator-(Instant lhs, Duration rhs);

    friend inline Duration operator-(Instant lhs, Instant rhs);

    explicit constexpr Duration(int64_t time_offset) : time_offset_(time_offset) {}

    int64_t time_offset_;

};

class Instant {
 public:
  explicit constexpr Instant(int64_t time) : time_(time) {}

  // Creates a new Time with an internal value of 0.  IsInitialized()
  // will return false for these times.
  static constexpr Instant zero() { return Instant(0); }

  // Creates a new Time with an infinite time.
  static constexpr Instant infinite() {
    return Instant(Duration::kInfiniteTimeUs);
  }

  Instant(const Instant &other) = default;

  Instant &operator=(const Instant &other) = default;

  // Produce the internal value to be used when logging.  This value
  // represents the number of microseconds since some epoch.  It may
  // be the UNIX epoch on some platforms.  On others, it may
  // be a CPU ticks based value.
  inline int64_t toDebuggingValue() const { return time_; }

  inline bool isInitialized() const { return 0 != time_; }

 private:

  friend inline bool operator==(Instant lhs, Instant rhs);

  friend inline bool operator<(Instant lhs, Instant rhs);

  friend inline Instant operator+(Instant lhs, Duration rhs);

  friend inline Instant operator-(Instant lhs, Duration rhs);

  friend inline Duration operator-(Instant lhs, Instant rhs);

  int64_t time_;
};


// Non-member relational operators for Time::Delta.
inline bool operator==(Duration lhs, Duration rhs) {
  return lhs.time_offset_ == rhs.time_offset_;
}
inline bool operator!=(Duration lhs, Duration rhs) {
  return !(lhs == rhs);
}
inline bool operator<(Duration lhs, Duration rhs) {
  return lhs.time_offset_ < rhs.time_offset_;
}
inline bool operator>(Duration lhs, Duration rhs) {
  return rhs < lhs;
}
inline bool operator<=(Duration lhs, Duration rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(Duration lhs, Duration rhs) {
  return !(lhs < rhs);
}
inline Duration operator<<(Duration lhs, size_t rhs) {
  return Duration(lhs.time_offset_ << rhs);
}
inline Duration operator>>(Duration lhs, size_t rhs) {
  return Duration(lhs.time_offset_ >> rhs);
}

// Non-member relational operators for Time.
inline bool operator==(Instant lhs, Instant rhs) {
  return lhs.time_ == rhs.time_;
}
inline bool operator!=(Instant lhs, Instant rhs) {
  return !(lhs == rhs);
}
inline bool operator<(Instant lhs, Instant rhs) {
  return lhs.time_ < rhs.time_;
}
inline bool operator>(Instant lhs, Instant rhs) {
  return rhs < lhs;
}
inline bool operator<=(Instant lhs, Instant rhs) {
  return !(rhs < lhs);
}
inline bool operator>=(Instant lhs, Instant rhs) {
  return !(lhs < rhs);
}

// Override stream output operator for gtest or CHECK macros.
inline std::ostream& operator<<(std::ostream& output, const Instant t) {
  output << t.toDebuggingValue();
  return output;
}

// Non-member arithmetic operators for Time::Delta.
inline Duration operator+(Duration lhs, Duration rhs) {
  return Duration(lhs.time_offset_ + rhs.time_offset_);
}
inline Duration operator-(Duration lhs, Duration rhs) {
  return Duration(lhs.time_offset_ - rhs.time_offset_);
}
inline Duration operator*(Duration lhs, int rhs) {
  return Duration(lhs.time_offset_ * rhs);
}
inline Duration operator*(Duration lhs, double rhs) {
  return Duration(
    static_cast<int64_t>(std::llround(lhs.time_offset_ * rhs)));
}
inline Duration operator*(int lhs, Duration rhs) {
  return rhs * lhs;
}
inline Duration operator*(double lhs, Duration rhs) {
  return rhs * lhs;
}

// Non-member arithmetic operators for Time and Time::Delta.
inline Instant operator+(Instant lhs, Duration rhs) {
  return Instant(lhs.time_ + rhs.time_offset_);
}
inline Instant operator-(Instant lhs, Duration rhs) {
  return Instant(lhs.time_ - rhs.time_offset_);
}
inline Duration operator-(Instant lhs, Instant rhs) {
  return Duration(lhs.time_ - rhs.time_);
}

// Override stream output operator for gtest.
inline std::ostream& operator<<(std::ostream& output,
                                const Duration delta) {
  output << delta.toDebugValue();
  return output;
}

#endif //SRC_BASE_TIME_BASE_H
