#pragma once
#include "cc/common.hpp"
#include "cc/arr-view.hpp"
#include "cc/hash.hpp"

class StrView {
 protected:
  char*  data_ = nullptr;
  size_t size_ = 0;

 public:
  using ValueType = char;

  inline static constexpr size_t npos = size_t(-1);

  StrView() = default;
  StrView(const char* str);
  StrView(const char* str, size_t size);

  bool        empty() const { return size_ == 0; }
  size_t      size() const { return size_; }
  char*       data() { return data_; }
  const char* data() const { return data_; }
  char*       begin() { return data_; }
  const char* begin() const { return data_; }
  char*       end() { return data_ + size_; }
  const char* end() const { return data_ + size_; }
  char&       operator[](size_t index) { return data_[index]; }
  const char& operator[](size_t index) const { return data_[index]; }

  bool operator==(StrView sv) const;
  bool operator!=(StrView sv) const { return !this->operator==(sv); }

  void assign(StrView other);

  bool             starts_with(char c) const;
  bool             ends_with(char c) const;
  bool             starts_with(StrView sv) const;
  bool             ends_with(StrView sv) const;
  ComparePos       compare(StrView sv) const;
  ComparePos       compare_ci(StrView sv) const;  // case-insensitive
  u64              hash() const;
  size_t           find(char c) const;
  size_t           find_last(char c) const;
  size_t           find(StrView term) const;
  size_t           find_last(StrView term) const;
  StrView          sub(size_t from, size_t count = UINT64_MAX) const;
  ArrView<StrView> split(char by, ArrView<StrView> out) const;
  ArrView<StrView> split_se(char by, ArrView<StrView> out) const;  // skip-empty
  ArrView<StrView> split(StrView by, ArrView<StrView> out) const;
  ArrView<StrView> split_se(StrView by, ArrView<StrView> out) const;  // skip-empty
  StrView          trim_left() const;
  StrView          trim_right() const;
  StrView          trim() const;
  StrView          to_lower();
  StrView          to_upper();
  bool             try_to_c_str(char* buf, size_t buffer_size) const;
  void             to_c_str(char* buf, size_t buffer_size) const;

  template <size_t buffer_size>
  bool try_to_c_str(char (&buf)[buffer_size]) const {
    return try_to_c_str(buf, buffer_size);
  }

  template <size_t buffer_size>
  void to_c_str(char (&buf)[buffer_size]) const {
    to_c_str(buf, buffer_size);
  }
};

inline StrView operator""_sv(const char* cstr, size_t size) {
  return {cstr, size};
}

template <size_t buffer_size>
[[nodiscard]] bool to_c_string(StrView val, char (&buf)[buffer_size]) {
  memset(buf, 0, buffer_size);
  if (val.size() > buffer_size - 1) {
    return false;
  }
  memcpy(buf, val.data(), val.size());
  return true;
}


class StrHash {
  u64 hash_ = 0;

 public:
  StrHash() = default;
  StrHash(StrView str);
  explicit constexpr StrHash(u64 hash) : hash_(hash) {}

  u64 hash() const { return hash_; }

  bool operator==(const StrHash& o) const { return hash_ == o.hash_; }
  bool operator!=(const StrHash& o) const { return hash_ != o.hash_; }
  bool operator<(const StrHash& o) const { return hash_ < o.hash_; }
};

inline constexpr StrHash operator""_sh(const char* cstr, size_t size) {
  return StrHash(cc::hash_fnv64(cstr, size));
}
