#include "cc/test.hpp"
#include "cc/sdict.hpp"

mTestCase(sdict) {
  SDict<u64, StrView> dict;
  dict.resize(3);

  dict.sort();
  mRequire(dict.find(1) == nullptr);

  dict.insert(3, "6");
  mRequireEqStr(dict[3], "6");
  mRequire(dict.find(4) == nullptr);

  dict.insert(2, "4");
  dict.sort();

  mRequire(dict.find(2) != nullptr);
  mRequireEqStr(dict[2], "4");
  mRequire(dict.find(3) != nullptr);
  mRequireEqStr(dict[3], "6");
  mRequire(dict.find(4) == nullptr);

  dict.insert(1, "2");
  dict.sort();

  mRequire(dict.find(0) == nullptr);
  mRequire(dict.find(1) != nullptr);
  mRequireEqStr(dict[1], "2");
  mRequire(dict.find(2) != nullptr);
  mRequireEqStr(dict[2], "4");
  mRequire(dict.find(3) != nullptr);
  mRequireEqStr(dict[3], "6");
  mRequire(dict.find(4) == nullptr);

  size_t iter = 0;
  for (const auto& [key, value] : dict) {
    if (key == 1) {
      mRequireEqStr(value, "2");
      ++iter;
    } else if (key == 2) {
      mRequireEqStr(value, "4");
      ++iter;
    } else if (key == 3) {
      mRequireEqStr(value, "6");
      ++iter;
    } else {
      mRequire(false);
    }
  }
  mRequire(iter == 3);
}
