#pragma once
#include <cc/common.hpp>

struct NullOpt {};

template <typename T>
class Opt {
  T    value_;
  bool is_set_ = false;

 public:
  Opt(NullOpt) {}

  Opt(T value) : value_(move(value)), is_set_(true) {}

  Opt(const Opt& other)            = default;
  Opt& operator=(const Opt& other) = default;

  Opt(Opt&& other) noexcept {
    swap(value_, other.value_);
    swap(is_set_, other.is_set_);
  }
  Opt& operator=(Opt&& other) noexcept {
    if (this != &other) {
      swap(value_, other.value_);
      swap(is_set_, other.is_set_);
    }
    return *this;
  }

  Opt& operator=(T&& value) noexcept {
    value_  = move(value);
    is_set_ = true;
    return *this;
  }

  Opt& operator=(const T& value) noexcept {
    value_  = value;
    is_set_ = true;
    return *this;
  }

  Opt& operator=(NullOpt) noexcept {
    value_  = {};
    is_set_ = false;
    return *this;
  }

  ~Opt() = default;

  T take() {
    assert(is_set_);
    is_set_ = false;
    return T(move(value_));
  }

  T& value() {
    assert(is_set_);
    return value_;
  }

  const T& value() const {
    assert(is_set_);
    return value_;
  }

  T* operator->() {
    assert(is_set_);
    return &value_;
  }

  const T* operator->() const {
    assert(is_set_);
    return &value_;
  }

  operator bool() const { return is_set_; }

  void reset() {
    is_set_ = false;
    value_  = {};
  }

  void set(T new_value) {
    value_  = move(new_value);
    is_set_ = true;
  }
};
