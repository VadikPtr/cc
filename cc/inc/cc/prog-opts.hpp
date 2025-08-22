#pragma once
#include "cc/common.hpp"
#include "cc/str.hpp"

class ProgOpts {
 public:
  enum Flags {
    Required,
    Optional,
  };

  enum class ArgumentType {
    S32,
    F32,
    Str,
  };

  struct ArgumentStr {
    const char* long_name;
    char        short_name;
    const char* help_argument = "???";
    const char* help_value    = "...";
    Str&        value;
    Flags       flags = Required;
  };

  struct ArgumentS32 {
    const char* long_name;
    char        short_name;
    const char* help_argument = "???";
    const char* help_value    = "...";
    s32&        value;
    Flags       flags = Required;
  };

  struct ArgumentF32 {
    const char* long_name;
    char        short_name;
    const char* help_argument = "???";
    const char* help_value    = "...";
    f32&        value;
    Flags       flags = Required;
  };

  struct Flag {
    const char* long_name;
    char        short_name;
    const char* help_argument = "???";
    bool&       value;
    Flags       flags = Required;
  };

  static void add(const Flag& arg);
  static void add(const ArgumentStr& arg);
  static void add(const ArgumentF32& arg);
  static void add(const ArgumentS32& arg);
  static void parse(int argc, const char** argv);

 private:
  static void add_argument(const char* long_name, char short_name,
                           const char* help_argument, const char* help_value, void* arg,
                           ArgumentType type, Flags flags);
};
