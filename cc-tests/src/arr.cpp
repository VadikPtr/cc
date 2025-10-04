#include "cc/test.hpp"
#include "cc/arr.hpp"
#include "cc/str.hpp"
#include "cc/algo.hpp"

namespace {
  class TestClass {
   public:
    bool is_unspecified         = true;
    bool is_default_constructed = false;
    bool is_special_constructed = false;

    TestClass() {
      is_unspecified         = false;
      is_default_constructed = true;
    }

    TestClass(int, int) {
      is_unspecified         = false;
      is_special_constructed = true;
    }
  };
}  // namespace

mTestCase(arr_default_ctor) {
  Arr<int> arr;
  mRequire(arr.empty());
  mRequire(arr.size() == 0);
  mRequire(arr.data() == nullptr);

  Arr<TestClass> arr2;
  mRequire(arr2.empty());
  mRequire(arr2.data() == nullptr);
}

mTestCase(arr_size_ctor) {
  Arr<int> arr(12);
  mRequire(!arr.empty());
  mRequire(arr.size() == 12);
  mRequire(arr.data() != nullptr);

  Arr<TestClass> arr2(3);
  mRequire(!arr2.empty());
  mRequire(arr2.size() == 3);
  mRequire(arr2.data() != nullptr);
  for (const auto& v : arr2) {
    mRequire(!v.is_unspecified);
    mRequire(v.is_default_constructed);
  }
}

mTestCase(arr_view_ctor) {
  int     values[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
  ArrView arr_view(values);
  mRequire(arr_view.size() == 10);
  mRequire(arr_view.data() != nullptr);

  Arr arr(arr_view);
  mRequire(arr.size() == 10);
  mRequire(arr.data() != nullptr);
  mRequire(arr.data() != arr_view.data());
  mRequire(arr.data() != values);
  for (size_t i = 0; i < arr.size(); ++i) {
    mRequire(arr[i] == (int)i + 1);
  }

  auto* old_arr = arr.data();
  arr           = ArrView(arr);
  mRequire(arr.size() == 10);
  mRequire(arr.data() == old_arr);
  for (size_t i = 0; i < arr.size(); ++i) {
    mRequire(arr[i] == (int)i + 1);
  }
}

mTestCase(arr_copy_assignment) {
  int      values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Arr      a1(ArrView{values});
  Arr<int> a2;
  a2 = a1;

  mRequire(a2.size() == 10);
  mRequire(a2.data() != nullptr);
  mRequire(a2.data() != a1.data());
  mRequire(a2.data() != values);
  for (size_t i = 0; i < a2.size(); ++i) {
    mRequire(a2[i] == (int)i);
  }

  int values2[] = {3, 4, 5};
  a2            = ArrView{values2};
  mRequire(a2.size() == 3);
  for (size_t i = 0; i < a2.size(); ++i) {
    mRequire(a2[i] == (int)i + 3);
  }
}

mTestCase(arr_move_ctor) {
  int  values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Arr  arr(ArrView{values});
  auto old_data = arr.data();
  auto old_size = arr.size();
  Arr  arr2(move(arr));
  mRequire(arr2.size() == old_size);
  mRequire(arr2.data() == old_data);
}

mTestCase(arr_move_assignment) {
  int      values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Arr<int> arr(values);
  auto     old_data = arr.data();
  auto     old_size = arr.size();
  Arr<int> arr2;
  mRequire(arr2.size() == 0);
  mRequire(arr2.data() == nullptr);
  arr2 = move(arr);
  mRequire(arr2.size() == old_size);
  mRequire(arr2.data() == old_data);
}

mTestCase(arr_resize_grow) {
  int values[] = {0, 1, 2, 3, 4};
  Arr arr(ArrView{values});
  arr.resize(10);
  mRequire(arr.size() == 10);
  mRequire(arr.data() != nullptr);
  for (size_t i = 0; i < 5; ++i) {
    mRequire(arr[i] == (int)i);
  }
  for (size_t i = 5; i < arr.size(); ++i) {
    // memory test
    arr[i] = 8;
    mRequire(arr[i] == 8);
  }
}

mTestCase(arr_resize_grow_complex) {
  Str values[] = {Str{"a"}, Str{"b"}, Str{"c"}};
  Arr arr(ArrView{values});
  arr.resize(10);
  mRequire(arr.size() == 10);
  mRequire(arr.data() != nullptr);
  mRequire(arr[0] == "a");
  mRequire(arr[1] == "b");
  mRequire(arr[2] == "c");
  for (size_t i = 3; i < arr.size(); ++i) {
    mRequire(arr[i].empty());
  }
}

mTestCase(arr_resize_shrink) {
  int values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Arr arr(ArrView{values});
  arr.resize(5);
  mRequire(arr.size() == 5);
  mRequire(arr.data() != nullptr);
  for (size_t i = 0; i < arr.size(); ++i) {
    mRequire(arr[i] == (int)i);
  }
}

mTestCase(arr_resize_zero) {
  int values[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
  Arr arr(ArrView{values});
  arr.resize(0);
  mRequire(arr.size() == 0);
  mRequire(arr.data() == nullptr);
  arr.resize(3);
  mRequire(arr.size() == 3);
  mRequire(arr.data() != nullptr);
}

mTestCase(arr_for_loop) {
  Arr<int> arr(8);
  for (auto& v : arr) {
    v = 12;
  }
  mRequire(arr.size() == 8);
  for (int i : arr) {
    mRequire(i == 12);
  }
}

mTestCase(arr_sort) {
  {
    int input[]    = {3, 2, 1};
    int expected[] = {1, 2, 3};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {5, -1, 4, 0};
    int expected[] = {-1, 0, 4, 5};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {42};
    int expected[] = {42};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    auto input    = ArrView<int>();
    auto expected = ArrView<int>();
    sort(input);
    mRequire(input == expected);
  }

  {
    auto input    = ArrView<int>();
    auto expected = ArrView<int>();
    sort(input, cc::is_greater<int>);
    mRequire(input == expected);
  }

  {
    int input[]    = {7, 7, 7, 7};
    int expected[] = {7, 7, 7, 7};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {9, 1, 8, 3, 2, 5};
    int expected[] = {1, 2, 3, 5, 8, 9};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {INT_MAX, INT_MIN, 0};
    int expected[] = {INT_MIN, 0, INT_MAX};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    StrView input[]    = {StrView("c"), StrView("b"), StrView("a")};
    StrView expected[] = {StrView("a"), StrView("b"), StrView("c")};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    StrView input[]    = {StrView("apple"), StrView("banana"), StrView("apricot")};
    StrView expected[] = {StrView("apple"), StrView("apricot"), StrView("banana")};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    StrView input[]    = {StrView("a")};
    StrView expected[] = {StrView("a")};
    sort(ArrView(input));
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {1, 2, 3};
    int expected[] = {3, 2, 1};
    sort(ArrView(input), cc::is_greater<int>);
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {5, -10, 0, 3};
    int expected[] = {5, 3, 0, -10};
    sort(ArrView(input), cc::is_greater<int>);
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {42};
    int expected[] = {42};
    sort(ArrView(input), cc::is_greater<int>);
    mRequire(ArrView(input) == ArrView(expected));
  }

  {
    int input[]    = {7, 7, 7};
    int expected[] = {7, 7, 7};
    sort(ArrView(input), cc::is_greater<int>);
    mRequire(ArrView(input) == ArrView(expected));
  }
}

mTestCase(ptr_intersection) {
  mRequire(!ptr_intersects(nullptr, 20, nullptr, 10));  // nullptr case
  mRequire(!ptr_intersects((void*)300, 20, (void*)100, 10));
  mRequire(ptr_intersects((void*)300, 20, (void*)290, 11));
  mRequire(!ptr_intersects((void*)300, 20, (void*)290, 10));
  mRequire(ptr_intersects((void*)300, 20, (void*)100, 500));
  mRequire(ptr_intersects((void*)300, 20, (void*)310, 2));
  mRequire(ptr_intersects((void*)300, 20, (void*)310, 200));
  mRequire(!ptr_intersects((void*)300, 20, (void*)400, 200));
}
