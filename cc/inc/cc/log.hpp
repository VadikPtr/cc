#pragma once
#include "cc/fs.hpp"
#include "cc/fmt.hpp"
#include "cc/time.hpp"
#include "cc/error.hpp"

enum class LogLevel {
  Debug,
  Info,
  Warn,
  Crit,
};

void log_set_level(LogLevel level);
void log_open_file();
void log_open_file(const Path& path);
bool log_is_enabled(LogLevel level);
void log_write(StrBuilder& builder);

#define mLogWrite(level, ...)                           \
  StrBuilder builder;                                   \
  fmt_timestamp(builder);                               \
  fmt(builder, " | ", level, " | ", __VA_ARGS__, '\n'); \
  log_write(builder);

#define mLogDebug(...)                   \
  if (log_is_enabled(LogLevel::Debug)) { \
    mLogWrite("debg", __VA_ARGS__);      \
  }

#define mLogInfo(...)                   \
  if (log_is_enabled(LogLevel::Info)) { \
    mLogWrite("info", __VA_ARGS__);     \
  }

#define mLogWarn(...)                   \
  if (log_is_enabled(LogLevel::Warn)) { \
    mLogWrite("WARN", __VA_ARGS__);     \
  }

#define mLogCrit(...)               \
  {                                 \
    mLogWrite("CRIT", __VA_ARGS__); \
    abort();                        \
  }
