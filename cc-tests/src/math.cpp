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

mTestCase(quat_test) {
  // Identity rotation
  Quat q1 = Quat(0, 0, 0, 1);
  // 90-degree rotation around Y
  Quat q2     = Quat(0, sin(g_pi / 4), 0, cos(g_pi / 4));
  Quat result = slerp(q1, q2, 0.5f);
  Vec3 vector = result.to_mat3() * Vec3(0, 0, -1);
  mLogInfo("vector: ", vector);
  mRequire(feq(vector, Vec3(-0.707, 0, -0.707), 0.01));
}
