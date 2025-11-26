#pragma once
#include <type_traits>
#include <new>
#include <cinttypes>
#include <cstdlib>
#include <cassert>
#include <cstring>
#include <cstdio>
#include <cmath>
#include <climits>

#ifdef _WIN32
  #define mPathSize 512
#else
  #define mPathSize PATH_MAX
#endif

#define mStringify(x) #x
#define mToString(x) mStringify(x)
#define mFileLine __FILE__ ":" mToString(__LINE__)
#define mTokenConcat1(x, y) x##y
#define mTokenConcat(x, y) mTokenConcat1(x, y)

#define mArrSize(v) sizeof(v) / sizeof(v[0])

#define mRuntimeAssert(...)                                          \
  if (!(__VA_ARGS__)) {                                              \
    fprintf(stderr, "Runtime assertion failed: " #__VA_ARGS__ "\n"); \
    fflush(stderr);                                                  \
    abort();                                                         \
  }

#define mMax(a, b) ((a) > (b) ? (a) : (b))
#define mMin(a, b) ((a) < (b) ? (a) : (b))
#define mClamp(x, a, b) ((x) < (a) ? (a) : ((x) > (b) ? (b) : (x)))

#if defined(DEBUG)
// #if 0
  #define mDebugTrap() __builtin_debugtrap()
  #define mDebugSwitch(debug, ndebug) debug
#else
  #define mDebugTrap()
  #define mDebugSwitch(debug, ndebug) ndebug
#endif

#ifdef __clang__
  #define mClangOnly(...) __VA_ARGS__
#else
  #define mClangOnly(...)
#endif

using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using s8  = int8_t;
using s16 = int16_t;
using s32 = int32_t;
using s64 = int64_t;
using f32 = float;
using f64 = double;

template <class T>
struct RemoveRef {
  using Type = T;
};

template <class T>
struct RemoveRef<T&> {
  using Type = T;
};

template <class T>
struct RemoveRef<T&&> {
  using Type = T;
};

template <class T>
struct RemoveConst {
  using Type = T;
};

template <class T>
struct RemoveConst<const T> {
  using Type = T;
};

template <class T>
using RemoveRefT = typename RemoveRef<T>::Type;

template <class T>
using RemoveConstT = typename RemoveConst<T>::Type;

template <class T>
using RemoveRefConstT = RemoveConstT<RemoveRefT<T>>;

template <typename T>
RemoveRefT<T>&& move(T&& x) noexcept {
  return static_cast<RemoveRefT<T>&&>(x);
}

template <class T>
T&& forward(RemoveRefT<T>& x) noexcept {
  return static_cast<T&&>(x);
}

template <class T>
T&& forward(RemoveRefT<T>&& x) noexcept {
  return static_cast<T&&>(x);
}

template <class T>
void swap(T& a, T& b) noexcept {
  T c(move(a));
  a = move(b);
  b = move(c);
}

template <typename T>
concept Equatable = requires(T t) {
  { t == t };  // Has operator==
};

template <typename T>
concept Comparable = requires(const T& t) {
  { t.compare(t) };
};

template <typename T>
concept OneSideComparable = requires(const T& t) {
  { t < t };
};

template <typename T>
concept Hashable = requires(T t) {
  { t.hash() };
};

template <class T>
concept PointerType = std::is_pointer_v<T>;

enum class ComparePos {
  Less,
  Equals,
  Greater,
};

template <typename T>
using OneSideComparer = bool (*)(const T& a, const T& b);

namespace cc {
  template <typename T>
  bool equals(const T& a, const T& b);

  template <Equatable T>
  bool equals(const T& a, const T& b) {
    return a == b;
  }

  template <typename T>
  bool is_less(const T& a, const T& b);

  template <Comparable T>
  bool is_less(const T& a, const T& b) {
    return a.compare(b) == ComparePos::Less;
  }

  template <OneSideComparable T>
  bool is_less(const T& a, const T& b) {
    return a < b;
  }

  template <typename T>
  bool is_greater(const T& a, const T& b);

  template <Comparable T>
  bool is_greater(const T& a, const T& b) {
    return a.compare(b) == ComparePos::Greater;
  }

  template <OneSideComparable T>
  bool is_greater(const T& a, const T& b) {
    return a > b;
  }
}  // namespace cc

template <typename TResult, typename THandle, TResult (*TOperation)(THandle)>
class FinalCleanup {
  THandle handle_;

 public:
  FinalCleanup(THandle handle) : handle_(handle) {}
  ~FinalCleanup() { TOperation(handle_); }
};

#define mFinalAction(variable, ...)                                                      \
  FinalCleanup<void, const RemoveRefConstT<decltype(variable)>&,                         \
               [](const RemoveRefConstT<decltype(variable)>& variable) { __VA_ARGS__; }> \
      mTokenConcat(cleanup_, __LINE__)(variable);

constexpr size_t operator""_kb(unsigned long long count) {
  return count * 1024;
}
constexpr size_t operator""_mb(unsigned long long count) {
  return count * 1024 * 1024;
}
constexpr size_t operator""_gb(unsigned long long count) {
  return count * 1024 * 1024 * 1024;
}

bool ptr_intersects(const void* a, size_t a_size, const void* b, size_t b_size);

template <typename T1, typename T2>
struct Pair {
  T1 first;
  T2 second;
};

template <class T>
struct DefaultDeleter {
  static void call(const T* ptr) { delete ptr; }
};

template <class T, class TDeleter = DefaultDeleter<T>>
class UPtr {
  T* ptr_ = nullptr;

 public:
  // construct
  UPtr() = default;
  UPtr(T* ptr) : ptr_(ptr) {}

  template <class TPtr>
  UPtr(TPtr* ptr) : ptr_(ptr) {}

  // copy
  UPtr(const UPtr&)            = delete;
  UPtr& operator=(const UPtr&) = delete;

  // move
  UPtr(UPtr&& o) noexcept { swap(ptr_, o.ptr_); }
  UPtr& operator=(UPtr&& o) noexcept {
    if (this != &o) {
      swap(ptr_, o.ptr_);
    }
    return *this;
  }

  ~UPtr() noexcept { reset(); }

  T*       get() { return ptr_; }
  const T* get() const { return ptr_; }

  void reset(T* ptr = nullptr) noexcept {
    if (ptr != ptr_) {
      TDeleter::call(ptr_);
    }
    ptr_ = ptr;
  }

  T* release() noexcept {
    T* tmp = ptr_;
    ptr_   = nullptr;
    return tmp;
  }

  template <class TPtr>
  UPtr<TPtr> cast() {
    return {static_cast<TPtr*>(release())};
  }

  operator bool() const noexcept { return ptr_ != nullptr; }
  T& operator*() const noexcept { return *ptr_; }
  T* operator->() const noexcept { return ptr_; }
};

// Raw movable pointer via swapping.
template <class T>
class RPtr {
  T* ptr_ = nullptr;

 public:
  // construct
  RPtr() = default;
  RPtr(T* ptr) : ptr_(ptr) {}

  // copy
  RPtr(const RPtr&)            = delete;
  RPtr& operator=(const RPtr&) = delete;

  // move
  RPtr(RPtr&& o) noexcept { swap(ptr_, o.ptr_); }
  RPtr& operator=(RPtr&& o) noexcept {
    if (this != &o) {
      swap(ptr_, o.ptr_);
    }
    return *this;
  }

  ~RPtr() noexcept = default;

  T*&      ref() { return ptr_; }
  T*       get() { return ptr_; }
  const T* get() const { return ptr_; }
  void     set(T* ptr) { ptr_ = ptr; }

  template <class TPtr>
  TPtr* as() {
    return static_cast<TPtr*>(ptr_);
  }

  operator T*() const { return ptr_; }
  operator bool() const { return ptr_ != nullptr; }
  T* operator->() const { return ptr_; }
  T& operator*() const { return *ptr_; }
};
