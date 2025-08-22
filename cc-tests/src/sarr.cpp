#include "cc/test.hpp"
#include "cc/sarr.hpp"
#include "cc/arr.hpp"

namespace {
  class TestClass {
   public:
    enum TestClassState {
      Unspecified,
      DefaultConstructed,
      SpecialConstructed,
    };

    TestClassState state;

    TestClass() { state = DefaultConstructed; }

    TestClass(int, int) { state = SpecialConstructed; }

    TestClass(TestClass&& other) noexcept {
      state       = other.state;
      other.state = Unspecified;
    }

    TestClass& operator=(TestClass&& other) noexcept {
      state       = other.state;
      other.state = Unspecified;
      return *this;
    }
  };
}  // namespace

mTestCase(sarr_default_ctor) {
  int  data[4];
  SArr arr{data};
  mRequire(arr.empty());
  mRequire(arr.size() == 0);
  mRequire(arr.data() != nullptr);

  TestClass tc[4];
  SArr      arr2{tc};
  mRequire(arr2.empty());
  mRequire(arr2.data() != nullptr);
  int real_count = 0;
  for ([[maybe_unused]] const auto& v : arr2) {
    real_count++;
  }
  mRequire(real_count == 0);
}

mTestCase(sarr_size_ctor) {
  int  data[4];
  SArr arr{data, 3};
  mRequire(!arr.empty());
  mRequire(arr.size() == 3);
  mRequire(arr.data() != nullptr);

  TestClass tc[4];
  SArr      arr2{tc, 2};
  mRequire(!arr2.empty());
  mRequire(arr2.size() == 2);
  mRequire(arr2.data() != nullptr);
  int real_count = 0;
  for (const auto& v : arr2) {
    mRequire(v.state == TestClass::DefaultConstructed);
    real_count++;
  }
  mRequire(real_count == 2);
}

mTestCase(sarr_copy) {
  int  source_data[] = {1, 2, 3};
  SArr arr{source_data, mArrSize(source_data)};
  SArr copy{arr};

  mRequire(!copy.empty());
  mRequire(copy.size() == 3);
  mRequire(copy.data() != nullptr);
  mRequire(copy[0] == 1);
  mRequire(copy[1] == 2);
  mRequire(copy[2] == 3);
  int real_count = 0;
  for ([[maybe_unused]] const auto& v : copy) {
    real_count++;
  }
  mRequire(real_count == 3);
}

mTestCase(sarr_push) {
  TestClass data[4];
  SArr      arr{data};

  arr.push(TestClass{1, 1});
  mRequire(!arr.empty());
  mRequire(arr.size() == 1);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::DefaultConstructed);
  mRequire(data[2].state == TestClass::DefaultConstructed);
  mRequire(arr[0].state == TestClass::SpecialConstructed);
  mRequire(arr.end() == arr.begin() + 1);

  arr.push(TestClass{1, 1});
  mRequire(!arr.empty());
  mRequire(arr.size() == 2);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::SpecialConstructed);
  mRequire(data[2].state == TestClass::DefaultConstructed);

  auto v = arr.pop();
  mRequire(v.state == TestClass::SpecialConstructed);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::Unspecified);
  mRequire(data[2].state == TestClass::DefaultConstructed);
}

mTestCase(sarr_dynamic) {
  Arr<TestClass> data{3};
  SArr           arr{data};

  arr.push(TestClass{1, 1});
  mRequire(!arr.empty());
  mRequire(arr.size() == 1);
  mRequire(arr.capacity() == 3);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::DefaultConstructed);
  mRequire(data[2].state == TestClass::DefaultConstructed);
  mRequire(arr[0].state == TestClass::SpecialConstructed);
  mRequire(arr.end() == arr.begin() + 1);

  arr.push(TestClass{1, 1});
  arr.push(TestClass{1, 1});
  mRequire(!arr.empty());
  mRequire(arr.size() == 3);
  mRequire(arr.capacity() == 3);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::SpecialConstructed);
  mRequire(data[2].state == TestClass::SpecialConstructed);

  auto v = arr.pop();
  mRequire(arr.size() == 2);
  mRequire(arr.capacity() == 3);
  mRequire(v.state == TestClass::SpecialConstructed);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::SpecialConstructed);
  mRequire(data[2].state == TestClass::Unspecified);

  arr.reset_storage(data.resize(5));
  mRequire(arr.size() == 2);
  mRequire(arr.capacity() == 5);
  mRequire(arr[0].state == TestClass::SpecialConstructed);
  mRequire(arr[1].state == TestClass::SpecialConstructed);
  mRequire(data[0].state == TestClass::SpecialConstructed);
  mRequire(data[1].state == TestClass::SpecialConstructed);
  mRequire(data[2].state == TestClass::Unspecified);
  mRequire(data[3].state == TestClass::DefaultConstructed);
  mRequire(data[4].state == TestClass::DefaultConstructed);
}
