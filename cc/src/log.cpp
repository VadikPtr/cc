#include "cc/log.hpp"

namespace {
  auto g_log_level =
#ifdef DEBUG
      LogLevel::Debug
#else
      LogLevel::Info
#endif
      ;

  File g_log_file;
}  // namespace

void log_set_level(LogLevel level) {
  g_log_level = level;
}

void log_open_file() {
  auto exe      = Path::to_exe().absolute();
  auto name     = Str(exe.name_without_ext()) + ".log";
  auto log_path = exe.parent() / name;
  mLogInfo("log path: ", log_path);
  log_open_file(log_path);
}

void log_open_file(const Path& path) {
  if (!g_log_file.try_open(path, "wb")) {
    fprintf(stderr, "Error: cannot open log file!\n");
  }
}

bool log_is_enabled(LogLevel level) {
  return level >= g_log_level;
}

void log_write(StrBuilder& builder) {
  auto line = builder.to_string();
  fwrite(line.data(), line.size(), 1, stdout);

  if (g_log_file.is_valid()) {
    if (!g_log_file.try_write_bytes(to_bytes(line))) {
      fprintf(stdout, "Error: cannot write log file\n");
    }
  }

  fflush(stdout);
}
