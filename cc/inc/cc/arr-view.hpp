#pragma once
#include "cc/common.hpp"

template <typename T>
class ArrView {
 protected:
  T*     data_ = nullptr;
  size_t size_ = 0;

 public:
  using ValueType = T;

  ArrView() = default;
  ArrView(T* data, size_t size) : data_(data), size_(size) {}

  template <size_t N>
  ArrView(T (&arr)[N]) : data_(arr), size_(N) {}

  bool     empty() const { return size_ == 0; }
  size_t   size() const { return size_; }
  size_t   byte_size() const { return size_ * sizeof(T); }
  T*       data() { return data_; }
  const T* data() const { return data_; }
  T*       begin() { return data_; }
  const T* begin() const { return data_; }
  T*       end() { return data_ + size_; }
  const T* end() const { return data_ + size_; }
  T&       operator[](size_t index) { return data_[index]; }
  const T& operator[](size_t index) const { return data_[index]; }

  bool operator==(ArrView o) const {
    if (size_ != o.size()) {
      return false;
    }
    for (size_t i = 0; i < size_; i++) {
      if (data_[i] != o.data_[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(ArrView o) const { return !this->operator==(o); }

  ArrView sub(size_t from, size_t count = UINT64_MAX) const {
    if (empty() || !count) {
      return {};
    }
    assert(size_ >= from);
    size_t max_count = size_ - from;
    if (count > max_count) {
      count = max_count;
    }
    return {data_ + from, count};
  }

  template <typename TOther>
  ArrView<TOther> cast() {
    return {reinterpret_cast<TOther*>(data_), sizeof(T) * size_ / sizeof(TOther)};
  }
};

template <typename T>
ArrView<u8> to_bytes(T& container) {
  return {reinterpret_cast<u8*>(container.data()),
          sizeof(typename T::ValueType) * container.size()};
}

template <typename T>
ArrView<u8> to_bytes(const T& container) {
  return {reinterpret_cast<u8*>(const_cast<typename T::ValueType*>(container.data())),
          sizeof(typename T::ValueType) * container.size()};
}

template <typename T>
void copy(ArrView<T> from, ArrView<T> to) {
  assert(from.size() == to.size());
  if constexpr (std::is_trivial_v<T>) {
    memcpy(to.data(), from.data(), sizeof(T) * to.size());
  } else {
    for (size_t i = 0; i < to.size(); ++i) {
      to[i] = from[i];
    }
  }
}

template <typename T>
void move(ArrView<T> from, ArrView<T> to) {
  assert(from.size() == to.size());
  if constexpr (std::is_trivial_v<T>) {
    memcpy(to.data(), from.data(), sizeof(T) * to.size());
  } else {
    for (size_t i = 0; i < to.size(); ++i) {
      to[i] = move(from[i]);
    }
  }
}
