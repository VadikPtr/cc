#pragma once
#include "cc/arr-view.hpp"

// Sized array view wrapper. Requires default ctor for T.
template <typename T>
class SArr {
  ArrView<T> storage_;
  size_t     size_ = 0;

 public:
  template <size_t N>
  SArr(T (&arr)[N]) : storage_(arr, N) {}

  template <size_t N>
  SArr(T (&arr)[N], size_t size) : storage_(arr, N), size_(size) {
    assert(size <= N);
  }

  SArr(ArrView<T> storage) : storage_(storage) {}
  SArr(ArrView<T> storage, size_t size) : storage_(storage), size_(size) {
    assert(size <= storage_.size());
  }

  // default behaviour is fine
  SArr()                                 = default;
  ~SArr() noexcept                       = default;
  SArr(const SArr& other)                = default;
  SArr& operator=(const SArr& other)     = default;
  SArr(SArr&& other) noexcept            = delete;  // cannot be implicitly moved
  SArr& operator=(SArr&& other) noexcept = delete;

  // replace storage in case of reallocating it
  void reset_storage(ArrView<T> storage) {
    assert(size_ <= storage.size());
    storage_ = storage;
  }

  bool       empty() const { return size_ == 0; }
  size_t     size() const { return size_; }
  size_t     byte_size() const { return size_ * sizeof(T); }
  size_t     capacity() const { return storage_.size(); }
  ArrView<T> view() { return {storage_.data(), size_}; }
  T*         data() { return storage_.data(); }
  const T*   data() const { return storage_.data(); }
  T*         begin() { return storage_.data(); }
  const T*   begin() const { return storage_.data(); }
  T*         end() { return storage_.data() + size_; }
  const T*   end() const { return storage_.data() + size_; }
  T&         operator[](size_t index) { return storage_[index]; }
  const T&   operator[](size_t index) const { return storage_[index]; }
  bool       operator==(SArr o) const { return view() == o.view(); }
  bool       operator!=(SArr o) const { return view() != o.view(); }

  T& push(T value) {
    assert(size_ < storage_.size());
    return storage_[size_++] = move(value);
  }

  T& push() {
    assert(size_ < storage_.size());
    return storage_[size_++];
  }

  T pop() {
    assert(size_ > 0);
    T result = move(storage_[--size_]);
    return result;
  }

  void resize(size_t new_size) {
    assert(new_size <= capacity());
    size_ = new_size;
  }
};
