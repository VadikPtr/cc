#include "cc/test.hpp"
#include "cc/all.hpp"

namespace {
  struct ThreadFuncEmpty final : ThreadFunc {
    bool run_called = false;

    ~ThreadFuncEmpty() override = default;

    const char* name() const override { return "TestEmpty"; }

    void run() override {
      mLogInfo("Empty Thread func");
      run_called = true;
    }
  };

  struct ThreadFuncInc final : ThreadFunc {
    u64&       value_;
    AtomicInt& value_atomic_;
    Mutex*     mutex_;

    ThreadFuncInc(u64& value, AtomicInt& value_atomic, Mutex* mutex)
        : value_(value), value_atomic_(value_atomic), mutex_(mutex) {}

    ~ThreadFuncInc() override = default;

    const char* name() const override { return "TestInc"; }

    void run() override {
      for (size_t i = 0; i < size_t(100'000); i++) {
        if (mutex_) {
          LockGuard g{*mutex_};
          ++value_;
        } else {
          ++value_;
          value_atomic_.fetch_add(1, MemoryOrder::Relaxed);
        }
      }
    }
  };

  struct ThreadFuncWaiter final : ThreadFunc {
    Mutex&             mutex_;
    ConditionVariable& cv_;
    bool&              ready_;

    ThreadFuncWaiter(Mutex& mutex, ConditionVariable& cv, bool& ready)
        : mutex_(mutex), cv_(cv), ready_(ready) {}

    ~ThreadFuncWaiter() override = default;

    const char* name() const override { return "TestWaiter"; }

    void run() override {
      auto begin = Time::now();
      LockGuard lock{mutex_};
      mLogInfo("Waiter: wait signal");
      while (!ready_) {
        cv_.wait(mutex_);
      }
      mLogInfo("Waiter: got signal in ", begin - Time::now());
    }
  };

  struct ThreadFuncNotify final : ThreadFunc {
    Mutex&             mutex_;
    ConditionVariable& cv_;
    bool&              ready_;

    ThreadFuncNotify(Mutex& mutex, ConditionVariable& cv, bool& ready)
        : mutex_(mutex), cv_(cv), ready_(ready) {}

    ~ThreadFuncNotify() override = default;

    const char* name() const override { return "TestNotify"; }

    void run() override {
      Thread::sleep(Time::make_secs(0.5));
      LockGuard lock{mutex_};
      mLogInfo("Notify: set ready");
      ready_ = true;
      cv_.notify_one();
    }
  };
}  // namespace

mTestCase(threads_empty) {
  Thread thread;
  mRequire(!thread.is_running());
  thread.start(new ThreadFuncEmpty());
  auto func = thread.join().cast<ThreadFuncEmpty>();
  mRequire(!thread.is_running());
  mRequire(func->run_called);

  auto func2 = thread.join();
  mRequire(!thread.is_running());
  mRequire(!func2);
}

mTestCase(threads_inc) {
  u64         result_no_mtx = 0;
  AtomicInt   result_atomic = 0;
  Arr<Thread> threads(8);

  for (auto& t : threads) {
    t.start(UPtr<ThreadFunc>(new ThreadFuncInc(result_no_mtx, result_atomic, nullptr)));
  }
  for (auto& t : threads) {
    t.join();
  }

  Mutex mutex;
  u64   result_mtx = 0;

  for (auto& t : threads) {
    t.start(UPtr<ThreadFunc>(new ThreadFuncInc(result_mtx, result_atomic, &mutex)));
  }
  for (auto& t : threads) {
    t.join();
  }

  mLogInfo("result_mtx = ", result_mtx);
  mLogInfo("result_no_mtx = ", result_no_mtx);
  mLogInfo("result_atomic = ", result_atomic.load(MemoryOrder::Relaxed));
  // note: valgrind does good job at sync threads...
  // mRequire(result_mtx != result_no_mtx);
  mRequire(result_mtx == size_t(100'000) * threads.size());
  mRequire(result_atomic.load() == 100'000 * int(threads.size()));
}

mTestCase(threads_cv) {
  Mutex             mutex;
  ConditionVariable cv;
  bool              ready = false;

  Thread waiter{new ThreadFuncWaiter{mutex, cv, ready}};
  Thread notify{new ThreadFuncNotify{mutex, cv, ready}};
}
