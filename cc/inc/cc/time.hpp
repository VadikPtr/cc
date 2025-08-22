#pragma once
#include "cc/fmt.hpp"
#include "cc/common.hpp"

void fmt_timestamp(StrBuilder& result);

// High performance time measurements. Operates on time since app start.
class Time {
  u64 value_ = 0;

 public:
  Time() = default;
  explicit Time(u64 value);

  static Time now();
  static Time make_secs(f64 val);
  static Time make_ms(f64 val);
  static Time make_us(f64 val);

  f64  secs() const;
  f64  ms() const;
  f64  us() const;  // microseconds
  f64  ns() const;
  u64  ticks() const;
  bool has_value() const;

  bool operator==(const Time& rhs) const;
  bool operator!=(const Time& rhs) const;

  Time  operator-(const Time& rhs) const;  // returns absolute difference
  Time& operator-=(const Time& rhs);       // returns absolute difference
  Time  operator+(const Time& rhs) const;
  Time& operator+=(const Time& rhs);
  bool  operator<(const Time& rhs) const;
  bool  operator>(const Time& rhs) const;

  Time abs_diff(const Time& other) const;
};

mFmtDeclare(Time);

class TimeDelta {
  Time begin_;
  Time delta_time_;
  u32  delta_ms_u_ = 0;
  f32  delta_ms_   = 0;
  f32  delta_us_   = 0;
  f32  delta_secs_ = 0;

 public:
  TimeDelta();
  void on_loop_end();

  u32  ms_u() const { return delta_ms_u_; }
  f32  ms() const { return delta_ms_; }
  f32  us() const { return delta_us_; }
  f32  secs() const { return delta_secs_; }
  Time time() const { return delta_time_; }
};

// simple profiler
class ScopedProfiler {
  StrView name_;
  Time    begin_;

 public:
  ScopedProfiler(StrView name);
  ~ScopedProfiler();
};

#define mScopedProfiler(name) \
  ScopedProfiler mTokenConcat(scoped_profiler_, __LINE__){name};
#ifdef _DEBUG
  #define mScopedProfilerDebug(name) mScopedProfiler(name)
#else
  #define mScopedProfilerDebug(...)
#endif


struct FrameProfilerStaticInfo {
  StrView name;
  Time    time_period = {};
  Time    time_begin  = {};
  size_t  count       = 0;
};

class FrameProfiler {
  FrameProfilerStaticInfo& info;

 public:
  FrameProfiler(FrameProfilerStaticInfo& info);
  ~FrameProfiler();
};

#define mFrameProfiler(name)                                                           \
  static FrameProfilerStaticInfo mTokenConcat(frame_profiler_static_, __LINE__){name}; \
  FrameProfiler                  mTokenConcat(frame_profiler_, __LINE__) {             \
    mTokenConcat(frame_profiler_static_, __LINE__)                    \
  }
