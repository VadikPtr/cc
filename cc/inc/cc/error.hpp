#pragma once
#include "cc/str.hpp"

class StackTrace {
  Str formatted_;

 public:
  StackTrace();

  StrView view() const { return formatted_; }
};

class Err : public std::exception {
  Str        what_;
  StackTrace trace_;

 public:
  Err() : Err(Str("Unknown error")) {}
  Err(Str what) : what_(move(what)) {}
  ~Err() override = default;

  StrView           message() const { return what_; }
  const StackTrace& stack_trace() const { return trace_; }
};
