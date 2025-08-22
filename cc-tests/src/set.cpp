#include "cc/test.hpp"
#include "cc/set.hpp"

struct CustomKey {
  int key;

  u64  hash() const { return cc::hash(key); }
  bool operator==(const CustomKey& o) const { return key == o.key; }
};

mTestCase(set_basic) {
  {
    Set<CustomKey> custom_set;
    mRequire(custom_set.size() == 0);
    custom_set.insert(CustomKey(1));
    custom_set.insert(CustomKey(2));
    custom_set.insert(CustomKey(3));
    mRequire(custom_set.size() == 3);
  }

  {
    Set<int> set;
    mRequire(set.size() == 0);
    set.insert(1);
    set.insert(2);
    set.insert(3);
    set.insert(3);
    set.insert(2);
    set.insert(1);

    size_t count = 0;
    for (auto key : set) {
      mRequire(key == 1 || key == 2 || key == 3);
      count++;
    }
    mRequire(set.size() == 3);
    mRequire(count == 3);

    count = 0;
    set.erase(2);
    for (auto key : set) {
      mRequire(key == 1 || key == 3);
      count++;
    }
    mRequire(set.size() == 2);
    mRequire(count == 2);

    mRequire(set.find(1) != set.end());
    mRequire(set.find(1).key() == 1);
    mRequire(*set.find(1) == 1);
    mRequire(set.find(2) == set.end());
  }
}
