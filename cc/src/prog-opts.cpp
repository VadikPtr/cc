#include "cc/prog-opts.hpp"
#include "cc/error.hpp"
#include "getopt/getopt.h"

namespace {
  struct ArgumentInfo {
    void*                  arg            = nullptr;
    ProgOpts::ArgumentType type           = ProgOpts::ArgumentType::Str;
    ProgOpts::Flags        flags          = ProgOpts::Flags::Optional;
    bool                   used           = false;
    bool                   value_provided = false;
  };

  struct FlagInfo {
    bool* flag = nullptr;
    bool  used = false;
  };

  constexpr size_t g_max_opts          = 32;
  bool             g_parsed            = false;
  size_t           g_opts_count        = 1;
  ArgumentInfo     g_args[g_max_opts]  = {};
  FlagInfo         g_flags[g_max_opts] = {};
  getopt_option_t  g_opts[g_max_opts]  =  //
      {{
          .name       = "help",
          .name_short = 'h',
          .type       = GETOPT_OPTION_TYPE_NO_ARG,
          .flag       = nullptr,
          .value      = 'h',
          .desc       = "Print this help text.",
          .value_desc = nullptr,
      }};

  [[noreturn]] void usage(getopt_context_t& ctx, int exit_code) {
    char buffer[2048];
    printf("Program arguments:\n%s",
           getopt_create_help_string(&ctx, buffer, sizeof(buffer)));
    fflush(stdout);
    exit(exit_code);
  }
}  // namespace

void ProgOpts::add_argument(const char* long_name, char short_name,
                            const char* help_argument, const char* help_value, void* arg,
                            ArgumentType type, Flags flags) {
  assert(!g_parsed);
  assert(g_opts_count + 1 < g_max_opts);

#if defined(DEBUG)
  for (size_t i = 0; i < g_opts_count; i++) {
    assert(strcmp(long_name, g_opts[i].name) != 0);
    assert(short_name != g_opts[i].name_short);
  }
#endif

  g_opts[g_opts_count].name       = long_name;
  g_opts[g_opts_count].name_short = int((unsigned char)short_name);
  g_opts[g_opts_count].flag       = nullptr;
  g_opts[g_opts_count].value      = int(g_opts_count);
  g_opts[g_opts_count].desc       = help_argument;
  g_opts[g_opts_count].value_desc = help_value;
  switch (type) {
    case ArgumentType::Str:
      g_opts[g_opts_count].type = GETOPT_OPTION_TYPE_REQUIRED;
      break;
    case ArgumentType::S32:
      g_opts[g_opts_count].type = GETOPT_OPTION_TYPE_REQUIRED_INT32;
      break;
    case ArgumentType::F32:
      g_opts[g_opts_count].type = GETOPT_OPTION_TYPE_REQUIRED_FP32;
      break;
  }

  g_args[g_opts_count].arg            = arg;
  g_args[g_opts_count].type           = type;
  g_args[g_opts_count].flags          = flags;
  g_args[g_opts_count].used           = true;
  g_args[g_opts_count].value_provided = false;

  g_opts_count++;
}

void ProgOpts::add(const Flag& arg) {
  assert(!g_parsed);
  assert(g_opts_count + 1 < g_max_opts);

#if defined(DEBUG)
  for (size_t i = 0; i < g_opts_count; i++) {
    assert(strcmp(arg.long_name, g_opts[i].name) != 0);
    assert(arg.short_name != g_opts[i].name_short);
  }
#endif

  g_opts[g_opts_count].name       = arg.long_name;
  g_opts[g_opts_count].name_short = int((unsigned char)arg.short_name);
  g_opts[g_opts_count].flag       = nullptr;
  g_opts[g_opts_count].value      = int(g_opts_count);
  g_opts[g_opts_count].desc       = arg.help_argument;
  g_opts[g_opts_count].value_desc = nullptr;

  g_flags[g_opts_count].flag = &arg.value;
  g_flags[g_opts_count].used = true;

  g_opts_count++;
}

void ProgOpts::add(const ArgumentStr& arg) {
  add_argument(arg.long_name, arg.short_name, arg.help_argument, arg.help_value,
               &arg.value, ArgumentType::Str, arg.flags);
}

void ProgOpts::add(const ArgumentF32& arg) {
  add_argument(arg.long_name, arg.short_name, arg.help_argument, arg.help_value,
               &arg.value, ArgumentType::F32, arg.flags);
}

void ProgOpts::add(const ArgumentS32& arg) {
  add_argument(arg.long_name, arg.short_name, arg.help_argument, arg.help_value,
               &arg.value, ArgumentType::S32, arg.flags);
}

void ProgOpts::parse(int argc, const char** argv) {
  assert(!g_parsed);
  g_parsed = true;

  getopt_context_t ctx;
  if (getopt_create_context(&ctx, argc, argv, g_opts) < 0) {
    throw Err("error while creating getopt ctx, bad options-list?"_s);
  }

  int opt;
  while ((opt = getopt_next(&ctx)) != -1) {
    switch (opt) {
      // case '+':
      //   mLogInfo("Got argument without flag: ", StrView(ctx.current_opt_arg));
      //   break;
      case '!':
        printf("Invalid usage of flag: %s\n", ctx.current_opt_arg);
        usage(ctx, 1);
      case '?':
        printf("Unknown flag: %s\n", ctx.current_opt_arg);
        usage(ctx, 1);
      case 'h':
        usage(ctx, 0);
      default:
        if (auto size_opt = size_t(opt); size_opt < g_opts_count) {
          if (auto& arg = g_args[size_opt]; arg.used) {
            switch (arg.type) {
              case ArgumentType::S32:
                *(s32*)arg.arg = ctx.current_value.i32;
                break;
              case ArgumentType::F32:
                *(f32*)arg.arg = ctx.current_value.fp32;
                break;
              case ArgumentType::Str:
                *(Str*)arg.arg = ctx.current_opt_arg;
                break;
              default:;
            }
            arg.value_provided = true;
            break;
          }
          if (auto& [flag, used] = g_flags[size_opt]; used) {
            *flag = true;
            break;
          }
        }

        printf("Unknown option: %d  (args count: %d)\n", int(opt), int(g_opts_count));
        usage(ctx, 1);
    }
  }

  for (size_t i = 0; i < g_opts_count; i++) {
    const auto& arg = g_args[i];
    if (arg.used && arg.flags == Required && !arg.value_provided) {
      printf("Error: argument --%s is required but not set!\n", g_opts[i].name);
      usage(ctx, 1);
    }
  }
}
