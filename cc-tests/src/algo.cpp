#include "cc/test.hpp"
#include "cc/arr.hpp"
#include "cc/algo.hpp"

mTestCase(algo_sort) {
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
