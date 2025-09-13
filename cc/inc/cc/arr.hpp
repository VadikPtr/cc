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

  Arr& operator+=(const Arr& other) {
    if (other.size_ > 0) {
      size_t insert_pos = size_;
      resize(size_ + other.size_, ResizeFlags::KeepOld);
      for (auto v : other) {
        data_[insert_pos++] = v;
      }
    }
    return *this;
  }

  Arr<Arr> split_into_groups(size_t groups_count) const {
    Arr<Arr> groups(groups_count);

    if (groups_count == 0) {
      return groups;
    }

    size_t group_size    = size_ / groups_count;
    size_t remainder     = size_ % groups_count;
    size_t start         = 0;
    size_t current_group = 0;

    for (size_t i = 0; i < groups_count; ++i) {
      size_t current_group_size = group_size + (i < remainder ? 1 : 0);
      groups[current_group++]   = this->sub(start, current_group_size);
      start += current_group_size;
    }

    return groups;
  }

  // very unoptimized push back. anyway it is nice to have
  void push(T value) {
    resize(size_ + 1, ResizeFlags::KeepOld);
    data_[size_ - 1] = move(value);
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
