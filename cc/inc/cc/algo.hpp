#pragma once
#include "cc/common.hpp"
#include "cc/arr-view.hpp"

template <typename T, typename TFunc>
void sort(ArrView<T> arr, TFunc&& compare_func) {
  if (arr.empty() || arr.size() == 1) {
    return;
  }
  for (size_t i = 0; i < arr.size(); ++i) {
    bool swapped = false;
    for (size_t j = 0; j < arr.size() - i - 1; ++j) {
      if (compare_func(arr[j + 1], arr[j])) {
        swapped = true;
        swap(arr[j], arr[j + 1]);
      }
    }
    if (!swapped) {  // array is sorted
      return;
    }
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
