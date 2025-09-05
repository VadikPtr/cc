#pragma once
#include "cc/arr.hpp"
#include "cc/threads.hpp"

namespace details {
  template <typename TValue>
  class ParallelForThread final : public ThreadFunc {
   public:
    struct Info {
      ArrView<TValue> values;
      Mutex           mutex;
      size_t          taken_jobs = 0;
      AtomicInt       failed     = 0;
      void (*value_func)(TValue&);
      void (*on_error)(TValue&, const Err& err);
    };

   private:
    Info& info_;

   public:
    explicit ParallelForThread(Info& info) : info_(info) {}
    ~ParallelForThread() override = default;

    void run() override {
      while (true) {
        TValue* value = nullptr;

        {
          LockGuard lg{info_.mutex};
          if (info_.taken_jobs == info_.values.size()) {
            return;
          }
          value = &info_.values[info_.taken_jobs++];
        }

        try {
          info_.value_func(*value);
        } catch (const Err& err) {
          if (info_.on_error) {
            info_.on_error(*value, err);
          }
          ++info_.failed;
        }
      }
    }
  };
}  // namespace details

template <typename TData>
int parallel_for(const Arr<TData>& data, void (*func)(TData&),
                 void (*on_error)(TData&, const Err& err) = nullptr) {
  Arr<Thread> threads(Thread::hardware_thread_count() - 1);
  typename details::ParallelForThread<TData>::Info info;
  info.values     = data;
  info.value_func = func;
  info.on_error   = on_error;

  for (auto& thread : threads) {
    thread.start(new details::ParallelForThread<TData>{info});
  }
  for (auto& thread : threads) {
    thread.join();
  }

  return info.failed;
}
