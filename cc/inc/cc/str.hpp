#pragma once
#include "cc/common.hpp"
#include "cc/str-view.hpp"


class Str;

class StrBuilder {
  char*  data_;
  size_t size_;
  size_t capacity_;
  char   small_buffer_[128];

 public:
  StrBuilder();
  ~StrBuilder() noexcept;

  // no movable
  StrBuilder(StrBuilder&& other)            = delete;
  StrBuilder& operator=(StrBuilder&& other) = delete;

  // no copyable
  StrBuilder(const StrBuilder&)            = delete;
  StrBuilder& operator=(const StrBuilder&) = delete;

  void appendf(const char* format, ...) mClangOnly(__attribute__((format(printf, 2, 3))));
  void append(StrView str);
  void append(char c);
  Str  to_string();
  StrView view() const;
  void    remove_last();
  void    reset();

 private:
  void init();
  void ensure_capacity(size_t capacity);
};


class Str : public StrView {
 public:
  // --- create
  Str() = default;
  explicit Str(size_t size);
  Str(size_t size, char ch);
  static Str from_raw(char* data, size_t size);

  // --- copy
  Str(const Str& o);
  explicit Str(const char* str);
  explicit Str(StrView str);
  Str(const char* str, size_t size);
  Str& operator=(const Str& o);
  Str& operator=(const char* str);
  Str& operator=(StrView str);

  // --- move
  Str(Str&& o) noexcept;
  Str& operator=(Str&& o) noexcept;

  ~Str();

  void resize(size_t required_size);
  Str& null_terminate();

  Str& operator+=(StrView o);

  template <typename... Args>
  static Str concat(const Args&... args) {
    Str  result{(args.size() + ...)};
    auto copy = [dst = result.data()](const auto& s) mutable {
      if (!s.empty()) {
        memcpy(dst, s.data(), s.size());
        dst += s.size();
      }
    };
    (copy(args), ...);
    return result;
  }
};

inline Str operator""_s(const char* cstr, size_t size) {
  return {cstr, size};
}

Str operator+(StrView a, StrView b);
