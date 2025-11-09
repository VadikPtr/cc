#pragma once
#include "cc/fs.hpp"
#include "cc/fmt.hpp"
#include "cc/time.hpp"

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
void log_write(LogLevel level, StrBuilder& builder);
void log_add_handler(void (*func)(LogLevel, StrView));

#define mLogWrite(level, text_level, ...)                    \
  StrBuilder builder;                                        \
  fmt_timestamp(builder);                                    \
  fmt(builder, " | ", text_level, " | ", __VA_ARGS__, '\n'); \
  log_write(level, builder);

#define mLogDebug(...)                               \
  if (log_is_enabled(LogLevel::Debug)) {             \
    mLogWrite(LogLevel::Debug, "debg", __VA_ARGS__); \
  }

#define mLogInfo(...)                               \
  if (log_is_enabled(LogLevel::Info)) {             \
    mLogWrite(LogLevel::Info, "info", __VA_ARGS__); \
  }

#define mLogWarn(...)                               \
  if (log_is_enabled(LogLevel::Warn)) {             \
    mLogWrite(LogLevel::Warn, "WARN", __VA_ARGS__); \
  }

#define mLogCrit(...)                               \
  {                                                 \
    mLogWrite(LogLevel::Crit, "CRIT", __VA_ARGS__); \
    abort();                                        \
  }
