#pragma once
#include "cc/common.hpp"
#include "cc/arr-view.hpp"

template <typename T, typename TFunc>
void sort(ArrView<T> arr, TFunc&& compare_func) {
  if (arr.size() <= 1) {
    return;
  }
  for (size_t i = 1; i < arr.size(); ++i) {
    T      key = arr[i];
    size_t j   = i;
    while (j > 0 && compare_func(key, arr[j - 1])) {
      arr[j] = arr[j - 1];
      --j;
    }
    arr[j] = key;
  }
}

template <typename T>
void sort(ArrView<T> arr, bool (*sort_func)(const T&, const T&)) {
  sort<T, decltype(sort_func)>(arr, move(sort_func));
}

template <typename T>
void sort(ArrView<T> arr) {
  sort(arr, cc::is_less<T>);
}
