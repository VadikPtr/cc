#include "cc/threads.hpp"
#include "cc/error.hpp"
#include "cc/log.hpp"

static_assert(__clang__,
              "This only works for clang, need adaptation for different compiler");

#if defined(_WIN32)
  #include <windows.h>
#else
  #include <pthread.h>
  #include <unistd.h>
  #include <sys/types.h>
  #ifdef __APPLE__
    #include <sys/sysctl.h>
  #endif
#endif

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


#if defined(_WIN32)
  ////////////////////////////////////////////////////////////////////////////////////
  //  WINDOWS IMPLS
  ////////////////////////////////////////////////////////////////////////////////////

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
      if (MultiByteToWideChar(CP_UTF8, 0, func->name(), -1, wname, mArrSize(wname)) ==
          0) {
        mLogWarn("Error set thread name: cannot convert name to wide encoding");
        return;
      }
      // Windows 10 1607+ only
      if (FAILED(SetThreadDescription(handle, wname))) {
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

  size_t platform_hardware_thread_count() {
    SYSTEM_INFO sys_info;
    GetSystemInfo(&sys_info);
    return sys_info.dwNumberOfProcessors <= 2 ? 2 : size_t(sys_info.dwNumberOfProcessors);
  }

  bool platform_pin_to_core() {
    u64       mask   = 1 << 0;  // bind to CPU 0 only
    HANDLE    thread = GetCurrentThread();
    DWORD_PTR result = SetThreadAffinityMask(thread, mask);
    return result != 0;
  }

  bool platform_prioritize() {
    return SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
  }

#elif defined(__APPLE__)
  ////////////////////////////////////////////////////////////////////////////////////
  //  APPLE IMPL
  ////////////////////////////////////////////////////////////////////////////////////

  void* unix_thread_func(void* arg) {
    auto* func = (ThreadFunc*)arg;
    if (func->name()) {
      pthread_setname_np(func->name());
    }
    func->run();
    return nullptr;
  }

  size_t platform_hardware_thread_count() {
    int    num_threads = 2;
    size_t size        = sizeof(num_threads);
    ::sysctlbyname("hw.logicalcpu", &num_threads, &size, nullptr, 0);
    return num_threads <= 2 ? 2 : size_t(num_threads);
  }

  bool platform_pin_to_core() {
    // apple cannot
    return false;
  }

  bool platform_prioritize() {
    // see https://gist.github.com/BrettRToomey/a1e32c2f99fbb9d34dd4bec8566e6d00
    // Set the QoS for the current thread to User Initiated (High Priority)
    // This is equivalent to setting a high "nice" value, but smarter
    int result = pthread_set_qos_class_self_np(QOS_CLASS_USER_INITIATED, 0);
    return result == 0;
  }

#elif defined(__linux__)
  ////////////////////////////////////////////////////////////////////////////////////
  //  LINUX IMPL
  ////////////////////////////////////////////////////////////////////////////////////

  void* unix_thread_func(void* arg) {
    auto* func = (ThreadFunc*)arg;
    if (func->name()) {
      char name[16] = {0};
      strncpy(name, func->name(), sizeof(name) - 1);
      if (pthread_setname_np(pthread_self(), name) != 0) {
        mLogWarn("Error set thread name");
      }
    }
    func->run();
    return nullptr;
  }

  size_t platform_hardware_thread_count() {
    cpu_set_t cpuset;
    sched_getaffinity(0, sizeof(cpuset), &cpuset);
    return size_t(CPU_COUNT(&cpuset));
  }

  bool platform_pin_to_core() {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);  // bind to cpu 0 only
    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    return result == 0;
  }

  void platform_prioritize() {
    // Lower nice value = higher priority. -20 is highest, 19 is lowest.
    // Requires root or CAP_SYS_NICE to go below 0.
    return nice(-10) != -1;
  }
#endif

#if defined(__APPLE__) or defined(__linux__)
  ////////////////////////////////////////////////////////////////////////////////////
  //  POSIX IMPLS
  ////////////////////////////////////////////////////////////////////////////////////

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

}  // namespace


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

size_t Thread::hardware_thread_count() {
  return platform_hardware_thread_count();
}

void Thread::pin_to_core_and_prioritize() {
  if (not platform_pin_to_core()) {
    mLogWarn("Pin current thread to core failed!");
  }
  if (not platform_prioritize()) {
    mLogWarn("Prioritize app failed!");
  }
  mLogDebug("Current thread pinned and prioritized");
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
