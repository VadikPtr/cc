#include "cc/test.hpp"
#include "cc/math.hpp"
#include "cc/log.hpp"

namespace {
  void converge_exp_check(f32 decay) {
    f32       a       = 10;
    f32       b       = 20;
    f32       dt_sum  = 0;
    const f32 dt_secs = 0.016f;

    do {
      a = exp_decay(a, b, decay, dt_secs);
      dt_sum += dt_secs;
      // mLogInfo("exp: ", a);
    } while (not feq(a, b, 0.2f));

    mLogInfo("exp decay done with param ", decay, " in ", dt_sum, " seconds");
  }
}  // namespace

mTestCase(exp_decay_test) {
  converge_exp_check(4);
  converge_exp_check(8);
  converge_exp_check(16);
}
