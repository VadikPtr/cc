#include "cc/process.hpp"
#include <stdlib.h>
#if !defined(_WIN32)
  #include <unistd.h>
#endif

void Process::exit(int code) {
  exit(code);
}

void Process::fast_exit(int code) {
  _exit(code);
}
