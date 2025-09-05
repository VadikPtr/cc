#pragma once
#include "cc/arr.hpp"
#include "cc/threads.hpp"

namespace details {
  template <typename TData>
  class ParallelForThread final : public ThreadFunc {
    Arr<TData> data_;
    void (*value_func_)(TData&);
    void (*on_error)(TData&, const Err& err);
    AtomicInt& failed_;

   public:
    ParallelForThread(Arr<TData> data, void (*value_func)(TData&),
                      void (*on_error)(TData&, const Err& err), AtomicInt& failed)
        : data_(move(data)),
          value_func_(value_func),
          on_error(on_error),
          failed_(failed) {}

    ~ParallelForThread() override = default;

    void run() override {
      for (auto& value : data_) {
        try {
          value_func_(value);
        } catch (const Err& err) {
          if (on_error) on_error(value, err);
          ++failed_;
        }
      }
    }
  };
}  // namespace details

template <typename TData>
int parallel_for(const Arr<TData>& data, void (*func)(TData&),
                 void (*on_error)(TData&, const Err& err) = nullptr) {
  Arr<Thread> threads(Thread::hardware_thread_count() - 1);
  size_t      current_thread = 0;
  AtomicInt   failed_count   = 0;

  // TODO: job stealing is better for this algo. but requires some syncro... :(
  for (auto&& chunk : data.split_into_groups(threads.size())) {
    auto* worker =
        new details::ParallelForThread<TData>{move(chunk), func, on_error, failed_count};
    threads[current_thread++].start(worker);
  }

  for (auto& thread : threads) {
    thread.join();
  }

  return failed_count.load();
}
