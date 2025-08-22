#pragma once
#include "cc/str.hpp"

class Err : public std::exception {
  Str what_;

 public:
  Err() : what_("Unknown error") {}
  Err(Str what) : what_(move(what)) {}
  ~Err() override = default;

  StrView message() const { return what_; }
};
