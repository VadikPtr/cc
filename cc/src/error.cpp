#include "cc/error.hpp"
#include "cc/log.hpp"
#include <execinfo.h>

void dump_stacktrace() {
  const int max_frames = 128;
  void*     callstack[max_frames];
  int       frames = backtrace(callstack, max_frames);
  char**    strs   = backtrace_symbols(callstack, frames);
  for (int i = 0; i < frames; ++i) {
    mLogInfo("backtrace[", i, "] ", StrView(strs[i]));
  }
  free(strs);
}
