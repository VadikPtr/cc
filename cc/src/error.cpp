#include "cc/error.hpp"
#include "cc/log.hpp"
#include <cxxabi.h>
#include <execinfo.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

// atos -o bin/debug/cupp 0x000000010001B5C8

namespace {
  class AtosPipe {
    ::FILE* file;
    ::pid_t pid;

   public:
    explicit AtosPipe(const char* exec_path, const char* addr) noexcept
        : file(0), pid(0) {
      char  prog_name[] = "/usr/bin/atos";
      char* argp[]      = {prog_name,
                           "-fullPath",
                           "-o",
                           const_cast<char*>(exec_path),
                           const_cast<char*>(addr),
                           0};

      int pdes[2];
      if (::pipe(pdes) < 0) {
        mLogWarn("pipe(pdes) fail");
        return;
      }

      pid = ::fork();
      switch (pid) {
        case pid_t(-1):
          perror("fork()");
          ::close(pdes[0]);
          ::close(pdes[1]);
          return;

        case 0:
          ::close(STDERR_FILENO);
          ::close(pdes[0]);
          if (pdes[1] != STDOUT_FILENO) {
            ::dup2(pdes[1], STDOUT_FILENO);
          }
          ::execv(prog_name, argp);
          ::exit(127);
      }

      file = ::fdopen(pdes[0], "r");
      if (!file) {
        mLogWarn("fdopen() fail");
      }
      ::close(pdes[1]);
    }

    operator ::FILE*() const noexcept { return file; }

    ~AtosPipe() noexcept {
      if (file) {
        ::fclose(file);
        int pstat = 0;
        ::kill(pid, SIGKILL);
        ::waitpid(pid, &pstat, 0);
      }
    }
  };

  Str extract_symbol(const char* exec_path, void* ptr) {
    StrBuilder addr;
    fmt(addr, Ptr{ptr}, '\0');

    AtosPipe pipe(exec_path, addr.view().data());
    if (!pipe) {
      return Str();
    }

    StrBuilder result;
    char       buf[128] = {0};
    while (!::feof(pipe)) {
      if (!::fgets(buf, sizeof(buf), pipe)) {
        break;
      }
      result.append(StrView(buf));
    }

    return Str(result.view().trim());
  }
}  // namespace

StackTrace::StackTrace() {
  const int max_frames = 128;
  void*     callstack[max_frames];
  int       num_frames = backtrace(callstack, max_frames);
  char**    symbols    = backtrace_symbols(callstack, num_frames);

  StrBuilder sb;

  // skip backtrace_symbols call, this ctor, Err ctor
  for (int i = 3; i < num_frames; ++i) {
    Dl_info info;
    int     r = dladdr(callstack[i], &info);
    if (r) {
#if 0
      int    status        = 0;
      char   buf[256]      = {0};
      size_t buf_size      = sizeof(buf);
      char* demangled_name = abi::__cxa_demangle(info.dli_sname, buf, &buf_size, &status);
      if (status == 0) {
        fmt(sb, Ptr{callstack[i]}, ' ', i, ": ", StrView(demangled_name), '\n');
        continue;
      }
#else
      Str symbol = extract_symbol(info.dli_fname, callstack[i]);
      if (!symbol.empty()) {
        fmt(sb, Ptr{callstack[i]}, ' ', i, ": ", symbol, '\n');
        continue;
      }
#endif
    }

    fmt(sb, StrView(symbols[i]), '\n');
  }

  free(symbols);
  formatted_ = sb.to_string();
}
