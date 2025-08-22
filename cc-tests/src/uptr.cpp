#include "cc/test.hpp"

namespace {
  class TestClass {
   public:
    int value = 0;
    explicit TestClass(int val) : value(val) {}
  };
}  // namespace

mTestCase(uptr_default_constructor) {
  UPtr<TestClass> ptr;
  mRequire(ptr.get() == nullptr);
}

mTestCase(uptr_make_unique) {
  auto ptr = UPtr(new TestClass(42));
  mRequire(ptr->value == 42);
}

mTestCase(uptr_reset) {
  auto ptr = UPtr(new TestClass(42));
  ptr.reset(new TestClass(84));
  mRequire(ptr->value == 84);
}

mTestCase(uptr_release) {
  auto       ptr    = UPtr(new TestClass(42));
  TestClass* rawPtr = ptr.release();
  mRequire(rawPtr != nullptr);
  mRequire(ptr.get() == nullptr);
  delete rawPtr;
}

mTestCase(uptr_move_semantics) {
  auto ptr1 = UPtr(new TestClass(42));
  UPtr ptr2(move(ptr1));
  mRequire(ptr1.get() == nullptr);
  mRequire(ptr2->value == 42);
}

mTestCase(uptr_deleter) {
  static bool deleted = false;

  struct Deleter {
    static void call(const TestClass* p) noexcept {
      delete p;
      deleted = true;
    }
  };

  auto ptr = UPtr<TestClass, Deleter>(new TestClass(42));
  ptr.reset();
  mRequire(deleted);
}
