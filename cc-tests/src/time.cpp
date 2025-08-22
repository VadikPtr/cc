#include "cc/test.hpp"
#include "cc/time.hpp"
#include "cc/log.hpp"
#include "cc/threads.hpp"

#if defined(__APPLE__)
static Time g_sleep_epsilon = Time::make_ms(200);
#else
static Time g_sleep_epsilon = Time::make_ms(50);
#endif

mTestCase(time_tests) {
  {
    auto now = Time::now();
    mLogInfo("now = ", now);

    auto sleep_time = Time::make_ms(500);
    Thread::sleep(sleep_time);
    auto sleep_recorded = Time::now() - now;

    mLogInfo("diff = ", sleep_recorded);
    mRequire(sleep_recorded - sleep_time < g_sleep_epsilon);
  }
}

mTestCase(time_delta_tests) {
  auto delta = TimeDelta{};
  mRequire(delta.ms_u() == 16);

  auto sleep_time = Time::make_ms(500);
  Thread::sleep(sleep_time);
  delta.on_loop_end();

  mLogInfo("diff = ", delta.time());
  mRequire(std::abs(f64(delta.ms()) - 500.0) < g_sleep_epsilon.ms());
  mRequire(delta.time() - sleep_time < g_sleep_epsilon);
}
