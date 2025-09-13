#pragma once
#include "cc/str.hpp"

void dump_stacktrace();

class Err : public std::exception {
  Str what_;

 public:
  Err() : Err(Str("Unknown error")) {}
  Err(Str what) : what_(move(what)) { dump_stacktrace(); }
  ~Err() override = default;

  StrView message() const { return what_; }
};
