#include "cc/error.hpp"
#include "cc/log.hpp"
#include <execinfo.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

namespace {
  class FD {
    int value_;

   public:
    FD(int value) : value_(value) {}

    void close() {
      if (value_ != -1) {
        ::close(value_);
        value_ = -1;
      }
    }

    operator int() const { return value_; }

    void redirect_to(FD& other) {
      ::dup2(other, value_);
      other.close();
    }
  };

  class Pipes {
    FD pipes[2] = {-1, -1};

   public:
    Pipes() {
      if (::pipe(reinterpret_cast<int*>(pipes)) < 0) {
        mLogWarn("pipe() fail");
        pipes[0] = pipes[1] = -1;
      }
    }

    ~Pipes() {
      if (is_open()) {
        ::close(pipes[0]);
        ::close(pipes[1]);
      }
    }

    bool is_open() const { return pipes[0] != -1 && pipes[1] != -1; }
    FD&  readfd() { return pipes[0]; }
    FD&  writefd() { return pipes[1]; }
  };

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

      Pipes pipes;
      if (!pipes.is_open()) {
        return;
      }

      pid = ::vfork();
      if (pid == pid_t(-1)) {
        perror("fork()");
        return;
      }

      if (pid == 0) {  // child
        pipes.readfd().close();
        FD(STDOUT_FILENO).redirect_to(pipes.writefd());
        FD(STDERR_FILENO).redirect_to(pipes.writefd());
        ::execv(prog_name, argp);
        ::exit(0);
      }

      pipes.writefd().close();
      file = ::fdopen(pipes.readfd(), "r");
      if (!file) {
        mLogWarn("fdopen() fail");
      }
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
      Str symbol = extract_symbol(info.dli_fname, callstack[i]);
      if (!symbol.empty()) {
        fmt(sb, Ptr{callstack[i]}, ' ', i, ": ", symbol, '\n');
        continue;
      }
    }
    fmt(sb, StrView(symbols[i]), '\n');
  }

  free(symbols);
  formatted_ = sb.to_string();
}
