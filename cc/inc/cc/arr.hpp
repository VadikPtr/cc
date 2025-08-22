#pragma once
#include "cc/common.hpp"
#include "cc/str-view.hpp"

enum class ResizeFlags {
  None    = 0,
  KeepOld = 1,
};

// Dynamic array. Requires default ctor for T.
template <typename T>
class Arr : public ArrView<T> {
 protected:
  using ArrView<T>::data_;
  using ArrView<T>::size_;

 public:
  Arr() = default;
  ~Arr() { delete[] data_; }

  // --- create

  explicit Arr(size_t size) { resize(size); }

  // --- copy

  Arr(const Arr& o) {
    resize(o.size());
    copy(o, *this);
  }

  explicit Arr(ArrView<T> arr) {
    resize(arr.size());
    copy(arr, *this);
  }

  Arr& operator=(const Arr& o) {
    if (this != &o) {
      resize(o.size(), ResizeFlags::None);
      copy(o, *this);
    }
    return *this;
  }

  Arr& operator=(ArrView<T> arr) {
    if (data_ != arr.data()) {
      assert(!ptr_intersects(data_, size_, arr.data(), arr.size()));
      resize(arr.size(), ResizeFlags::None);
      copy(arr, *this);
    }
    return *this;
  }

  // --- move

  Arr(Arr&& o) noexcept {
    swap(data_, o.data_);
    swap(size_, o.size_);
  }

  Arr& operator=(Arr&& o) noexcept {
    if (this != &o) {
      swap(data_, o.data_);
      swap(size_, o.size_);
    }
    return *this;
  }

  // --- misc

  Arr& resize(size_t required_size, ResizeFlags flags = ResizeFlags::KeepOld) {
    data_ = resize_buffer(required_size, flags);
    size_ = required_size;
    return *this;
  }

 private:
  T* resize_buffer(size_t required_size, ResizeFlags flags);
};

// --- details

template <typename T>
T* Arr<T>::resize_buffer(size_t required_size, ResizeFlags flags) {
  if (required_size == size_) {
    return data_;
  }
  if (required_size == 0) {
    delete[] data_;
    return nullptr;
  }
  auto* new_data = new T[required_size];
  if (size_t move_count = mMin(size_, required_size);
      move_count && ((int)flags & (int)ResizeFlags::KeepOld) != 0) {
    auto from = ArrView(data_, move_count);
    auto to   = ArrView(new_data, move_count);
    move(from, to);
  }
  delete[] data_;
  return new_data;
}
