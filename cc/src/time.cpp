#include "cc/time.hpp"

#include "cc/log.hpp"

#if defined(_WIN32)
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif
  #include <windows.h>
#elif defined(__APPLE__)
  #include <mach/mach_time.h>
#endif

#if !defined(_WIN32)
  #include <sys/time.h>
  #include <time.h>
#endif


namespace {

  // prevent 64-bit overflow when computing relative timestamp
  [[maybe_unused]] s64 int64_muldiv(s64 value, s64 numer, s64 denom) {
    s64 q = value / denom;
    s64 r = value % denom;
    return q * numer + r * numer / denom;
  }

  struct State {
#if defined(_WIN32)
    LARGE_INTEGER freq  = {};
    LARGE_INTEGER start = {};

    State() {
      QueryPerformanceFrequency(&freq);
      QueryPerformanceCounter(&start);
    }

    u64 now() const {
      LARGE_INTEGER qpc = {};
      QueryPerformanceCounter(&qpc);
      // return (u64)int64_muldiv(qpc.QuadPart - start.QuadPart, 1'000'000'000,
      //                          freq.QuadPart);
      decltype(LARGE_INTEGER::QuadPart) elapsed{};
      elapsed = qpc.QuadPart - start.QuadPart;
      elapsed *= 1'000'000'000;
      elapsed /= freq.QuadPart;
      return u64(elapsed);
    }

#elif defined(__APPLE__)
    u64 start = 0;

    State() { start = clock_gettime_nsec_np(CLOCK_UPTIME_RAW); }

    u64 now() const { return clock_gettime_nsec_np(CLOCK_UPTIME_RAW) - start; }

#else  // linux
    u64 start = 0;

    State() {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      start = (u64)ts.tv_sec * 1000000000 + (u64)ts.tv_nsec;
    }

    u64 now() const {
      struct timespec ts;
      clock_gettime(CLOCK_MONOTONIC, &ts);
      return ((u64)ts.tv_sec * 1000000000 + (u64)ts.tv_nsec) - start;
    }
#endif

  } g_state;

}  // namespace


void fmt_timestamp(StrBuilder& result) {
#ifdef _WIN32
  SYSTEMTIME st = {};
  GetLocalTime(&st);
  fmt(result,  //
      st.wYear, '-', ZeroPrefixU16(2, st.wMonth), '-', ZeroPrefixU16(2, st.wDay), 'T',
      ZeroPrefixU16(2, st.wHour), ':', ZeroPrefixU16(2, st.wMinute), ':',
      ZeroPrefixU16(2, st.wSecond), '.', ZeroPrefixU16(3, st.wMilliseconds));
#else
  struct timeval tv = {};
  gettimeofday(&tv, NULL);
  time_t    now = tv.tv_sec;
  struct tm st  = {};
  localtime_r(&now, &st);
  fmt(result,  //
      u16(st.tm_year + 1900), '-', ZeroPrefixU16(2, u16(st.tm_mon + 1)), '-',
      ZeroPrefixU16(2, u16(st.tm_mday)), 'T', ZeroPrefixU16(2, u16(st.tm_hour)), ':',
      ZeroPrefixU16(2, u16(st.tm_min)), ':', ZeroPrefixU16(2, u16(st.tm_sec)), '.',
      ZeroPrefixU16(3, u16(tv.tv_usec / 1000)));
#endif
}


Time::Time(u64 value) : value_{value} {}

Time Time::now() {
  return Time{g_state.now()};
}

Time Time::make_secs(f64 val) {
  f64 x = val * 1'000'000'000.0;
  if (x < 0) return Time{};
  return Time{u64(x)};
}

Time Time::make_ms(f64 val) {
  f64 x = val * 1'000'000.0;
  if (x < 0) return Time{};
  return Time{u64(x)};
}

Time Time::make_us(f64 val) {
  f64 x = val * 1'000.0;
  if (x < 0) return Time{};
  return Time{u64(x)};
}

f64 Time::secs() const {
  return (f64)value_ / 1'000'000'000.0;
}
f64 Time::ms() const {
  return (f64)value_ / 1'000'000.0;
}
f64 Time::us() const {
  return (f64)value_ / 1'000.0;
}
f64 Time::ns() const {
  return (f64)value_;
}
u64 Time::ticks() const {
  return value_;
}
bool Time::has_value() const {
  return value_ != 0;
}
bool Time::operator==(const Time& rhs) const {
  return value_ == rhs.value_;
}
bool Time::operator!=(const Time& rhs) const {
  return value_ != rhs.value_;
}
Time Time::operator-(const Time& rhs) const {
  if (value_ > rhs.value_) {
    return Time{value_ - rhs.value_};
  }
  return Time{rhs.value_ - value_};
}
Time& Time::operator-=(const Time& rhs) {
  return *this = *this - rhs;
}
Time Time::operator+(const Time& rhs) const {
  return Time{value_ + rhs.value_};
}
Time& Time::operator+=(const Time& rhs) {
  return *this = *this + rhs;
}
bool Time::operator<(const Time& rhs) const {
  return value_ < rhs.value_;
}
bool Time::operator>(const Time& rhs) const {
  return value_ > rhs.value_;
}
f32 Time::operator/(const Time& rhs) const {
  assert(rhs.has_value());
  return (f32)value_ / rhs.value_;
}
Time Time::abs_diff(const Time& other) const {
  if (value_ > other.value_) {
    return Time{value_ - other.value_};
  }
  return Time{other.value_ - value_};
}

mFmtImpl(Time) {
  if (!v.has_value()) {
    out.append("0");
    return;
  }
  if (v.ticks() < 1'000llu) {
    Fmt<u64>::format(v.ticks(), out);
    out.append("ns");
    return;
  }
  if (v.ticks() < 1'000'000llu) {
    Fmt<f64>::format(v.us(), out);
    out.append("us");
    return;
  }
  if (v.ticks() < 1'000'000'000llu) {
    Fmt<f64>::format(v.ms(), out);
    out.append("ms");
    return;
  }
  Fmt<f64>::format(v.secs(), out);
  out.append("s");
}

TimeDelta::TimeDelta() {
  begin_      = Time::now();
  delta_time_ = Time::make_ms(16.66666667);
  delta_ms_   = f32(delta_time_.ms());
  delta_us_   = f32(delta_time_.us());
  delta_secs_ = f32(delta_time_.secs());
  delta_ms_u_ = u32(delta_ms_);
}

void TimeDelta::on_loop_end() {
  Time now    = Time::now();
  delta_time_ = now - begin_;
  delta_ms_   = f32(delta_time_.ms());
  delta_us_   = f32(delta_time_.us());
  delta_secs_ = f32(delta_time_.secs());
  delta_ms_u_ = u32(delta_ms_);
  begin_      = now;
}

ScopedProfiler::ScopedProfiler(StrView name) : name_(name), begin_(Time::now()) {}

ScopedProfiler::~ScopedProfiler() {
  auto length = Time::now() - begin_;
  mLogInfo("scope[", name_, "]: ", length);
}

FrameProfiler::FrameProfiler(FrameProfilerStaticInfo& info) : info(info) {
  info.time_begin = Time::now();
}

FrameProfiler::~FrameProfiler() {
  info.count++;
  info.time_period += Time::now() - info.time_begin;
  if (info.time_period.secs() >= 1 || info.count > 1000) {
    f64 ms  = info.time_period.ms() / (f64)info.count;
    u64 fps = u64((f64)info.count / info.time_period.secs());
    mLogInfo("frame[", info.name, "] ", ms, "ms (", fps, "fps)");
    info.count       = 0;
    info.time_period = {};
  }
}
