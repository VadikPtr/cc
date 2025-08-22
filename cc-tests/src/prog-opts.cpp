#include "cc/test.hpp"
#include "cc/prog-opts.hpp"
#include "cc/log.hpp"

mTestCase(prog_opts_example) {
  Str  amogus;
  bool flag = false;

  ProgOpts::add(ProgOpts::ArgumentStr{
      .long_name     = "amogus",
      .short_name    = 'a',
      .help_argument = "Abobus help.",
      .value         = amogus,
      .flags         = ProgOpts::Required,
  });
  ProgOpts::add(ProgOpts::Flag{
      .long_name     = "flag",
      .short_name    = 'f',
      .help_argument = "Test flag.",
      .value         = flag,
  });

  const char* args[] = {
      "program_name.exe",
      "--amogus=abobus",
      "-f",
  };
  ProgOpts::parse(mArrSize(args), args);

  mLogInfo("amogus = ", amogus);
  mLogInfo("flag = ", flag);
}
