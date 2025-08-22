#pragma once
#include "cc/common.hpp"
#include "cc/time.hpp"


struct ThreadFunc {
  // destructor will be called on joiner thread normally, on creator thread when error
  virtual ~ThreadFunc() = default;

  // preferable not bigger than 15 chars (linux)
  virtual const char* name() const { return nullptr; }

  virtual void run() = 0;
};

class Thread {
  void*            handle_ = nullptr;
  UPtr<ThreadFunc> func_;

 public:
  Thread() = default;
  explicit Thread(UPtr<ThreadFunc> func);
  Thread(Thread&& other) noexcept;
  Thread& operator=(Thread&& other) noexcept;
  ~Thread();

  // no copyable
  Thread(const Thread&)            = delete;
  Thread& operator=(const Thread&) = delete;

  bool             is_running() const { return handle_ != nullptr; }
  void             start(UPtr<ThreadFunc> func);
  UPtr<ThreadFunc> join();

  static void sleep(Time time);
};


class Mutex {
  alignas(8) struct {
    u8 bytes[64];
  } data_;

 public:
  Mutex();
  ~Mutex();

  Mutex(const Mutex&)            = delete;
  Mutex& operator=(const Mutex&) = delete;
  Mutex(Mutex&&)                 = delete;
  Mutex& operator=(Mutex&&)      = delete;

  bool try_lock();
  void lock();
  void unlock();

  friend class ConditionVariable;
};


// Usage:
//   LockGuard lock(mtx);
//   while (!ready) {
//     cv.wait(lock);
//   }
class ConditionVariable {
  alignas(8) struct {
    u8 bytes[64];
  } data_;

 public:
  ConditionVariable();
  ~ConditionVariable();

  void wait(Mutex& mutex);
  void notify_one();
  void notify_all();
};


class LockGuard {
  Mutex& mutex_;
  bool   owns_;

 public:
  explicit LockGuard(Mutex& m);
  ~LockGuard();

  void lock();
  void unlock();

  LockGuard(const LockGuard&)            = delete;
  LockGuard& operator=(const LockGuard&) = delete;
};


enum class MemoryOrder {
  Relaxed,  // no sync
  Consume,
  Acquire,
  Release,
  AcquireRelease,
  SequentialConsistency,
};

class AtomicInt {
  int value_;

 public:
  AtomicInt(int v = 0) : value_(v) {}

  // Relaxed, Release, SequentialConsistency
  void store(int v, MemoryOrder mo = MemoryOrder::Release);

  // Relaxed, Acquire, SequentialConsistency
  int load(MemoryOrder mo = MemoryOrder::Acquire) const;

  int  fetch_add(int v, MemoryOrder mo = MemoryOrder::SequentialConsistency);
  int  fetch_sub(int v, MemoryOrder mo = MemoryOrder::SequentialConsistency);
  bool compare_exchange_strong(int& expected, int desired,
                               MemoryOrder mo = MemoryOrder::SequentialConsistency);

  operator int() const { return load(); }
  AtomicInt& operator++();
  AtomicInt& operator--();
};
