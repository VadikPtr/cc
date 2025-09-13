#pragma once
#include "cc/common.hpp"
#include "cc/str.hpp"


#define mRequire(cond)                                        \
  if (!(cond)) {                                              \
    mDebugTrap();                                             \
    throw TestCaseFail(mFileLine "  [" #cond "] is false\n"); \
  }

#define mRequireEqBytes(result, expected, count) \
  mRequire(memcmp(result, expected, count) == 0)
#define nRequireNeqBytes(result, expected, count) \
  mRequire(memcmp(result, expected, count) != 0)

#define mRequireEqStr(result, expected)                                           \
  if (Str a{result}, b{expected}; a != b) {                                       \
    throw TestCaseFail(mFileLine "\n  '%.*s'\n    !=\n  '%.*s'\n", (int)a.size(), \
                       a.data(), (int)b.size(), b.data());                        \
  }

#define mRequireNeqStr(result, expected)                                          \
  if (Str a{result}, b{expected}; a == b) {                                       \
    throw TestCaseFail(mFileLine "\n  '%.*s'\n    ==\n  '%.*s'\n", (int)a.size(), \
                       a.data(), (int)b.size(), b.data());                        \
  }

#define mTestCase(func)                                  \
  static void     func();                                \
  static TestCase mTokenConcat(g_test_case_, __LINE__) = \
      register_test_case(TestCase{#func, func});         \
  static void func()


class TestCaseFail final : public std::exception {
  char* message;

 public:
  explicit TestCaseFail(const char* fmt, ...);
  ~TestCaseFail() override;
  const char* what() const noexcept override;
};

struct TestCase {
  const char* name;
  void (*func)();
};


TestCase register_test_case(TestCase test_case);
int      tests_main(int argc, const char** argv);
