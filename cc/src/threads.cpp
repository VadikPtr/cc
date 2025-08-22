#include "cc/threads.hpp"
#include "cc/error.hpp"
#include "cc/log.hpp"

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <pthread.h>
  #include <unistd.h>
#endif

#ifdef __clang__
namespace {
  int to_builtin_memory_order(MemoryOrder order) {
    switch (order) {
      case MemoryOrder::Relaxed:
        return __ATOMIC_RELAXED;
      case MemoryOrder::Consume:
        return __ATOMIC_CONSUME;
      case MemoryOrder::Acquire:
        return __ATOMIC_ACQUIRE;
      case MemoryOrder::Release:
        return __ATOMIC_RELEASE;
      case MemoryOrder::AcquireRelease:
        return __ATOMIC_ACQ_REL;
      case MemoryOrder::SequentialConsistency:
        return __ATOMIC_SEQ_CST;
      default:
        return __ATOMIC_SEQ_CST;  // fallback
    }
  }
}
#endif


#if defined(_WIN32)

DWORD WINAPI win_thread_func(LPVOID arg) {
  auto* func = (ThreadFunc*)arg;
  func->run();
  return 0;
}

void thread_create(void*& handle, ThreadFunc* func) {
  handle = CreateThread(nullptr, 0, win_thread_func, func, 0, nullptr);
  if (handle == nullptr) {
    throw Err("Error creating thread"_s);
  }

  if (func->name()) {
    wchar_t wname[64];
    if (MultiByteToWideChar(CP_UTF8, 0, func->name(), -1, wname, mArrSize(wname)) == 0) {
      mLogWarn("Error set thread name: cannot convert name to wide encoding");
    } else if (FAILED(SetThreadDescription(handle, wname))) {  // Windows 10 1607+ only
      mLogWarn("Error set thread name");
    }
  }
}

void thread_join(void*& handle) {
  if (WaitForSingleObject(handle, INFINITE) == WAIT_OBJECT_0) {
    CloseHandle(handle);
  } else {
    mLogWarn("Error joining thread");
  }
}

void thread_sleep(u32 ms) {
  Sleep(ms);
}

#else

void* unix_thread_func(void* arg) {
  auto* func = (ThreadFunc*)arg;

  if (func->name()) {
  #if defined(__APPLE__)
    pthread_setname_np(func->name());
  #else
    char name[16] = {};
    strncpy(name, func->name(), sizeof(name) - 1);
    if (pthread_setname_np(pthread_self(), name) != 0) {
      mLogWarn("Error set thread name");
    }
  #endif
  }

  func->run();
  return nullptr;
}

void thread_create(void*& handle, ThreadFunc* func) {
  if (pthread_create((pthread_t*)&handle, nullptr, unix_thread_func, func) != 0) {
    throw Err("Error creating thread"_s);
  }
}

void thread_join(void*& handle) {
  pthread_join((pthread_t)handle, nullptr);
}

void thread_sleep(u32 ms) {
  usleep(ms * 1000);
}

#endif


Thread::Thread(UPtr<ThreadFunc> func) {
  start(move(func));
}

Thread::~Thread() {
  join();
}

Thread::Thread(Thread&& other) noexcept {
  swap(handle_, other.handle_);
  swap(func_, other.func_);
}

Thread& Thread::operator=(Thread&& other) noexcept {
  if (this != &other) {
    swap(handle_, other.handle_);
    swap(func_, other.func_);
  }
  return *this;
}

void Thread::start(UPtr<ThreadFunc> func) {
  if (handle_ != nullptr) {
    throw Err("Thread is already started"_s);
  }
  func_ = move(func);
  thread_create(handle_, func_.get());
}

UPtr<ThreadFunc> Thread::join() {
  if (is_running()) {
    thread_join(handle_);
    handle_ = nullptr;
  }
  return move(func_);
}

void Thread::sleep(Time time) {
  thread_sleep(u32(time.ms()));
}


#if defined(_WIN32)

Mutex::Mutex() : data_{} {
  static_assert(sizeof(data_) >= sizeof(CRITICAL_SECTION));
  static_assert(alignof(Mutex) >= alignof(CRITICAL_SECTION));
  auto* m = &(CRITICAL_SECTION&)data_;
  InitializeCriticalSection(m);
}
Mutex::~Mutex() {
  auto* m = &(CRITICAL_SECTION&)data_;
  DeleteCriticalSection(m);
}
bool Mutex::try_lock() {
  auto* m = &(CRITICAL_SECTION&)data_;
  return TryEnterCriticalSection(m) == TRUE;
}
void Mutex::lock() {
  auto* m = &(CRITICAL_SECTION&)data_;
  EnterCriticalSection(m);
}
void Mutex::unlock() {
  auto* m = &(CRITICAL_SECTION&)data_;
  LeaveCriticalSection(m);
}

ConditionVariable::ConditionVariable() : data_{} {
  static_assert(sizeof(data_) >= sizeof(CONDITION_VARIABLE));
  static_assert(alignof(ConditionVariable) >= alignof(CONDITION_VARIABLE));
  auto* cv = &(CONDITION_VARIABLE&)data_;
  InitializeConditionVariable(cv);
}
ConditionVariable::~ConditionVariable() {}
void ConditionVariable::wait(Mutex& mutex) {
  auto* cv = &(CONDITION_VARIABLE&)data_;
  auto* m  = &(CRITICAL_SECTION&)mutex.data_;
  SleepConditionVariableCS(cv, m, INFINITE);
}
void ConditionVariable::notify_one() {
  auto* cv = &(CONDITION_VARIABLE&)data_;
  WakeConditionVariable(cv);
}
void ConditionVariable::notify_all() {
  auto* cv = &(CONDITION_VARIABLE&)data_;
  WakeAllConditionVariable(cv);
}

#else

Mutex::Mutex() : data_{} {
  static_assert(sizeof(data_) >= sizeof(pthread_mutex_t));
  static_assert(alignof(Mutex) >= alignof(pthread_mutex_t));
  auto*                m = &(pthread_mutex_t&)data_;
  [[maybe_unused]] int r = pthread_mutex_init(m, nullptr);
  assert(r == 0);
}
Mutex::~Mutex() {
  auto* m = &(pthread_mutex_t&)data_;
  pthread_mutex_destroy(m);
}
bool Mutex::try_lock() {
  auto* m = &(pthread_mutex_t&)data_;
  return pthread_mutex_trylock(m) == 0;
}
void Mutex::lock() {
  auto* m = &(pthread_mutex_t&)data_;
  pthread_mutex_lock(m);
}
void Mutex::unlock() {
  auto* m = &(pthread_mutex_t&)data_;
  pthread_mutex_unlock(m);
}

ConditionVariable::ConditionVariable() : data_{} {
  static_assert(sizeof(data_) >= sizeof(pthread_cond_t));
  static_assert(alignof(ConditionVariable) >= alignof(pthread_cond_t));
  auto*                cv = &(pthread_cond_t&)data_;
  [[maybe_unused]] int r  = pthread_cond_init(cv, nullptr);
  assert(r == 0);
}
ConditionVariable::~ConditionVariable() {
  auto* cv = &(pthread_cond_t&)data_;
  pthread_cond_destroy(cv);
}
void ConditionVariable::wait(Mutex& mutex) {
  auto* cv = &(pthread_cond_t&)data_;
  auto* m  = &(pthread_mutex_t&)mutex.data_;
  pthread_cond_wait(cv, m);
}
void ConditionVariable::notify_one() {
  auto* cv = &(pthread_cond_t&)data_;
  pthread_cond_signal(cv);
}
void ConditionVariable::notify_all() {
  auto* cv = &(pthread_cond_t&)data_;
  pthread_cond_broadcast(cv);
}

#endif

LockGuard::LockGuard(Mutex& mutex) : mutex_(mutex) {
  mutex_.lock();
  owns_ = true;
}
LockGuard::~LockGuard() {
  if (owns_) {
    mutex_.unlock();
  }
}
void LockGuard::lock() {
  if (!owns_) {
    mutex_.lock();
    owns_ = true;
  }
}
void LockGuard::unlock() {
  if (owns_) {
    mutex_.unlock();
    owns_ = false;
  }
}

#ifdef __clang__

void AtomicInt::store(int v, MemoryOrder mo) {
  __atomic_store_n(&value_, v, to_builtin_memory_order(mo));
}
int AtomicInt::load(MemoryOrder mo) const {
  return __atomic_load_n(&value_, to_builtin_memory_order(mo));
}
int AtomicInt::fetch_add(int v, MemoryOrder mo) {
  return __atomic_fetch_add(&value_, v, to_builtin_memory_order(mo));
}
int AtomicInt::fetch_sub(int v, MemoryOrder mo) {
  return __atomic_fetch_sub(&value_, v, to_builtin_memory_order(mo));
}
bool AtomicInt::compare_exchange_strong(int& expected, int desired, MemoryOrder mo) {
  return __atomic_compare_exchange_n(&value_, &expected, desired,
                                     /*weak=*/false, to_builtin_memory_order(mo),
                                     to_builtin_memory_order(mo));
}
AtomicInt& AtomicInt::operator++() {
  fetch_add(1);
  return *this;
}
AtomicInt& AtomicInt::operator--() {
  fetch_sub(1);
  return *this;
}

#endif