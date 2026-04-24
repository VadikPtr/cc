#include "cc/log.hpp"
#include "cc/test.hpp"
#include "cc/all.hpp"

mTestCase(log_open_test) {
  Path log_path = Path::to_exe().parent().absolute() / "engine.log"_sv;
  mLogInfo("Log path: ", log_path);
  log_open_file(log_path);
  mLogDebug("Hello from debug!");
  mLogInfo("Hello from info!");
  mLogWarn("Hello from warn!");
  mLogInfo("Stack: ", StackTrace().view());
  // do not call crit. it will crash app
}

mTestCase(log_handler_test) {
  static List<Str> lines;
  void (*func)(LogLevel, StrView) = [](LogLevel, StrView view) {
    lines.push_back(Str(view));
  };
  log_add_handler(func);
  mLogInfo("Hello!");
  log_remove_handler(func);

  mRequire(lines.size() == 1) Str log_line = *lines.begin();
  log_line = Str(log_line.sub(log_line.find_last('|') + 2));
  mRequireEqStr(log_line, "Hello!\n");
}
