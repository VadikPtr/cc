#include "cc/test.hpp"
#include "cc/dict.hpp"
#include "cc/str.hpp"
#include "cc/fmt.hpp"

namespace {
  struct CustomKey {
    int key;

    u64  hash() const { return cc::hash(key); }
    bool operator==(const CustomKey& o) const { return key == o.key; }
  };

  struct CustomValue {
    int value;
  };
}  // namespace


mTestCase(dict_custom) {
  Dict<CustomKey, CustomValue> di;

  mRequire(di.insert({1}, {2}) != di.end());

  auto it = di.find({1});
  mRequire(it != di.end());
  mRequire(it.key().key == 1);
  mRequire(it.value().value == 2);

  mRequire(di.find({3}) == di.end());
  it = di.insert({3}, {4});
  mRequire(it != di.end());
  mRequire(it.key().key == 3);
  mRequire(it.value().value == 4);

  mRequire(di.size() == 2);
}

mTestCase(dict_abstract) {
  Dict<int, int> di;

  mRequire(di.insert(1, 2) != di.end());

  auto it = di.find(1);
  mRequire(it != di.end());
  mRequire(it.key() == 1);
  mRequire(it.value() == 2);

  mRequire(di.find(3) == di.end());
  it = di.insert(3, 4);
  mRequire(it != di.end());
  mRequire(it.key() == 3);
  mRequire(it.value() == 4);

  mRequire(di.size() == 2);
}

mTestCase(dict_str) {
  Dict<Str, int> di;

  mRequire(di.insert(Str("1"), 1) != di.end());

  auto it = di.find(Str("1"));
  mRequire(it != di.end());
  mRequire(it.key() == StrView{"1"});
  mRequire(it.value() == 1);

  mRequire(di.find(Str("3")) == di.end());
  it = di.insert(Str("3"), 4);
  mRequire(it != di.end());
  mRequire(it.key() == StrView{"3"});
  mRequire(it.value() == 4);

  mRequire(di.size() == 2);

  mRequire(di.erase(Str("1")));
  mRequire(di.size() == 1);
  mRequire(di.find(Str("1")) == di.end());
  mRequire(di.find(Str("3")) != di.end());
}

mTestCase(dict_str_stress) {
  Dict<Str, Str> di;

  constexpr int count = 1024;

  di.reserve(256);

  for (int i = 0; i < count; i++) {
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(di.find(key) == di.end());
    mRequire(di.insert(Str(key), Str(val)) != di.end());
    mRequire(di.find(key) != di.end());
    mRequire(di.find(key).key() == key);
    mRequire(di.find(key).value() == val);
  }

  di.shrink();
  di.reserve(4096);
  di.shrink();

  for (int i = 0; i < count; i++) {
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(di.find(key) != di.end());
    mRequire(di.find(key).key() == key);
    mRequire(di.find(key).value() == val);
  }

  for (int i = 0; i < count; i++) {
    if (i % 2 != 0) {
      continue;
    }
    auto key = fmt(i);
    mRequire(di.erase(key));
  }

  for (int i = 0; i < count; i++) {
    if (i % 2 != 1) {
      continue;
    }
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(di.find(key) != di.end());
    mRequire(di.find(key).key() == key);
    mRequire(di.find(key).value() == val);
  }

  auto copy = di;
  Dict copy2(copy);
  for (int i = 0; i < count; i++) {
    if (i % 2 != 1) {
      continue;
    }
    auto key = fmt(i);
    auto val = fmt(i * 2);

    mRequire(copy.find(key) != copy.end());
    mRequire(copy2.find(key) != copy2.end());
    mRequire(di.find(key) != di.end());

    mRequire(copy.find(key).key() == key);
    mRequire(copy2.find(key).key() == key);
    mRequire(di.find(key).key() == key);
    mRequire(&di.find(key).key() != &copy.find(key).key());
    mRequire(&copy2.find(key).key() != &copy.find(key).key());

    mRequire(copy.find(key).value() == val);
    mRequire(copy2.find(key).value() == val);
    mRequire(di.find(key).value() == val);
    mRequire(&di.find(key).value() != &copy.find(key).value());
    mRequire(&copy2.find(key).value() != &copy.find(key).value());
  }

  di.shrink();
  for (int i = 0; i < count; i++) {
    if (i % 2 != 1) {
      continue;
    }
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(di.find(key) != di.end());
    mRequire(di.find(key).key() == key);
    mRequire(di.find(key).value() == val);
  }

  size_t it_count = 0;
  for (auto it = di.begin(); it != di.end(); ++it) {
    char buf[32] = {};
    memcpy(buf, it.key().data(), it.key().size());
    int key = (int)strtol(buf, nullptr, 10);
    memset(buf, 0, sizeof(buf));
    memcpy(buf, it.value().data(), it.value().size());
    int val = (int)strtol(buf, nullptr, 10);
    mRequire(key * 2 == val);
    mRequire(key % 2 != 0);
    it_count++;
  }
  mRequire(di.size() == it_count);

  Dict<Str, Str> other;
  other.insert(Str("1"), Str("2"));
  other.insert(Str("3"), Str("4"));
  other.insert(Str("5"), Str("6"));
  other = move(di);

  int iter_count = 0;
  for (int i = 0; i < count; i++) {
    if (i % 2 != 1) {
      continue;
    }
    iter_count++;
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(other.find(key) != other.end());
    mRequire(other.find(key).key() == key);
    mRequire(other.find(key).value() == val);
  }

  mRequire(other.size() == (size_t)iter_count);

  Dict other2(move(other));
  for (int i = 0; i < count; i++) {
    if (i % 2 != 1) {
      continue;
    }
    iter_count++;
    auto key = fmt(i);
    auto val = fmt(i * 2);
    mRequire(other2.find(key) != other2.end());
    mRequire(other2.find(key).key() == key);
    mRequire(other2.find(key).value() == val);
  }
}
