#include "cc/test.hpp"
#include "cc/all.hpp"

mTestCase(log_open_test) {
  auto log_path = Path::to_exe().parent().absolute() / "engine.log"_sv;
  mLogInfo("Log path: ", log_path);
  log_open_file(log_path);
  mLogDebug("Hello from debug!");
  mLogInfo("Hello from info!");
  mLogWarn("Hello from warn!");
  // do not call crit. it will crash app
}
