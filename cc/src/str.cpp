#include "cc/str.hpp"
#include "cc/common.hpp"
#include <cstdarg>

namespace {
  char* str_alloc(size_t size) {
    auto result = static_cast<char*>(malloc(size));
    mRuntimeAssert(result != nullptr);
    // printf("alloc: %p (%u)\n", result, (u32)size);
    return result;
  }

  char* str_realloc(char* str, size_t size) {
    auto result = static_cast<char*>(realloc(str, size));
    mRuntimeAssert(result != nullptr);
    // printf("realloc: %p\n", result);
    return result;
  }

  void str_free(char* str) {
    // printf("free: %p\n", str);
    free(str);
  }
}  // namespace


StrBuilder::StrBuilder() {
  init();
}

StrBuilder::~StrBuilder() noexcept {
  if (data_ != small_buffer_) {
    free(data_);
  }
}

// StrBuilder::StrBuilder(StrBuilder&& other) noexcept {
//   swap(data_, other.data_);
//   swap(size_, other.size_);
//   swap(capacity_, other.capacity_);
// }
//
// StrBuilder& StrBuilder::operator=(StrBuilder&& other) noexcept {
//   if (this != &other) {
//     swap(data_, other.data_);
//     swap(size_, other.size_);
//     swap(capacity_, other.capacity_);
//   }
//   return *this;
// }

void StrBuilder::appendf(const char* format, ...) {
  va_list args;

  va_start(args, format);
  int char_count = vsnprintf(nullptr, 0, format, args);
  va_end(args);

  if (char_count >= 0) {
    auto size = size_t(char_count) + 1;
    ensure_capacity(size_ + size);
    va_start(args, format);
    vsnprintf(data_ + size_, size, format, args);
    va_end(args);
    size_ += size_t(char_count);
  }
}

void StrBuilder::append(StrView str) {
  if (!str.empty()) {
    ensure_capacity(size_ + str.size());
    memcpy(data_ + size_, str.data(), str.size());
    size_ += str.size();
  }
}

void StrBuilder::append(char c) {
  ensure_capacity(size_ + 1);
  data_[size_] = c;
  ++size_;
}

Str StrBuilder::to_string() {
  if (data_ == small_buffer_) {
    return {data_, size_};
  }
  auto res = Str::from_raw(data_, size_);
  init();
  return res;
}

StrView StrBuilder::view() const {
  return {data_, size_};
}

void StrBuilder::init() {
  data_     = small_buffer_;
  size_     = 0;
  capacity_ = sizeof(small_buffer_);
}

void StrBuilder::ensure_capacity(size_t capacity) {
  constexpr size_t alloc_step     = 32;
  size_t           capacity_alloc = (capacity & ~(alloc_step - 1)) + alloc_step;
  if (capacity_alloc <= capacity_) {
    return;
  }
  if (data_ == small_buffer_) {
    data_ = (char*)malloc(capacity_alloc);
    memcpy(data_, small_buffer_, size_);
  } else {
    data_ = (char*)realloc(data_, capacity_alloc);
  }
  capacity_ = capacity_alloc;
}


Str::~Str() {
  str_free(data_);
}

Str::Str(const Str& o) {
  if (o.size_) {
    resize(o.size_);
    memcpy(data_, o.data_, size_);
  }
}

Str& Str::operator=(const Str& o) {
  if (this != &o) {
    resize(o.size_);
    if (data_) memcpy(data_, o.data_, size_);
  }
  return *this;
}

Str& Str::operator=(const char* str) {
  if (data_ != str) {
    size_t size = strlen(str);
    assert(!ptr_intersects(data_, size_, str, size));
    resize(size);
    if (data_) memcpy(data_, str, size_);
  }
  return *this;
}

Str& Str::operator=(StrView str) {
  if (data_ != str.data()) {
    assert(!ptr_intersects(data_, size_, str.data(), str.size()));
    resize(str.size());
    if (data_) memcpy(data_, str.data(), size_);
  }
  return *this;
}

Str::Str(Str&& o) noexcept {
  swap(data_, o.data_);
  swap(size_, o.size_);
}

Str& Str::operator=(Str&& o) noexcept {
  if (this != &o) {
    swap(data_, o.data_);
    swap(size_, o.size_);
  }
  return *this;
}

Str::Str(size_t size) {
  resize(size);
}

Str::Str(const char* str) {
  if (str != nullptr) {
    if (size_t len = strlen(str)) {
      resize(len);
      memcpy(data_, str, size_);
    }
  }
}

Str::Str(StrView str) {
  if (str.data()) {
    resize(str.size());
    memcpy(data_, str.data(), size_);
  }
}

Str::Str(const char* str, size_t size) {
  if (str != nullptr && size) {
    resize(size);
    memcpy(data_, str, size_);
  }
}

Str::Str(size_t size, char ch) {
  if (size) {
    resize(size);
    memset(data_, ch, size);
  }
}

Str Str::from_raw(char* data, size_t size) {
  Str res;
  res.data_ = data;
  res.size_ = size;
  return res;
}

void Str::resize(size_t required_size) {
  if (required_size == 0) {
    str_free(data_);
    data_ = nullptr;
  } else {
    if (size_ == 0) {
      data_ = str_alloc(required_size);
    } else if (required_size != size_) {
      data_ = str_realloc(data_, required_size);
    }
  }
  size_ = required_size;
}

Str& Str::null_terminate() {
  resize(size_ + 1);
  data_[size_ - 1] = 0;
  return *this;
}

Str& Str::operator+=(StrView o) {
  if (o.empty()) {
    return *this;
  }
  assert(!ptr_intersects(data_, size_, o.data(), o.size()));
  if (empty()) {
    return *this = o;
  }
  size_t old_size = size_;
  resize(size() + o.size());
  memcpy(data_ + old_size, o.data(), o.size());
  return *this;
}

Str operator+(StrView a, StrView b) {
  Str res(a.size() + b.size());
  if (!a.empty()) memcpy(res.data(), a.data(), a.size());
  if (!b.empty()) memcpy(res.data() + a.size(), b.data(), b.size());
  return res;
}
