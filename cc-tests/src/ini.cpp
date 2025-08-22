#include "cc/test.hpp"
#include "cc/all.hpp"

mTestCase(ini_parse) {
  auto ini = Ini::parse(R"(
global = test

[Section to remove]
a = b
c = d

[Section]
a = 1
b=2
c =3
d  =  4
arr = [1, 2, 3]
  hello  =  wrold \
hmmm
 wrold  = test
last = check \
)");

  for (auto section : ini) {
    mLogDebug("section: ", section.name());
    for (auto prop : section) {
      mLogDebug("  prop ", prop.name(), ": ", prop.value());
    }
  }

  mRequire(!ini["Unknown Section"]);
  mRequire(!ini["Unknown Section"]["Unknown prop"_sv]);
  mRequire(ini["none"]["unknown"_sv].value_or(2) == 2);
  mRequireEqStr(ini["Section"]["a"_sv].value(), "1"_sv);
  mRequire(ini["Section"]["b"_sv].value<u64>() == u64(2));
  mRequire(ini["Section"]["b"_sv].value_or<u64>(4) == u64(2));
  mRequireEqStr(ini["Section"]["c"_sv].value<Str>(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"_sv].value<StrView>(), "4"_sv);
  mRequireEqStr(ini["Section"]["hello"_sv].value(), "wrold \\\nhmmm"_sv);
  mRequireEqStr(ini["Section"]["hello"_sv].value<StrWithWrap>().str, "wrold \nhmmm"_sv);
  mRequireEqStr(ini["Section"]["wrold"_sv].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"_sv].value(), "check \\"_sv);

  int arr[3] = {};
  ini["Section"]["arr"_sv].value(arr);
  mRequire(arr[0] == 1);
  mRequire(arr[1] == 2);
  mRequire(arr[2] == 3);

  ini["Section"]["hello"_sv].remove();
  mRequire(!ini["Section"]["hello"_sv]);
  mRequireEqStr(ini["Section"]["a"_sv].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"_sv].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"_sv].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"_sv].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"_sv].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"_sv].value(), "check \\"_sv);

  mRequire(ini["Section to remove"]);
  ini["Section to remove"].remove();
  mRequire(!ini["Section to remove"]);
  mRequireEqStr(ini["Section"]["a"_sv].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"_sv].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"_sv].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"_sv].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"_sv].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"_sv].value(), "check \\"_sv);

  Ini copy;
  copy = ini;
  mRequireEqStr(ini["Section"]["a"_sv].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"_sv].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"_sv].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"_sv].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"_sv].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"_sv].value(), "check \\"_sv);

  mRequireEqStr(copy["Section"]["a"_sv].value(), "1"_sv);
  mRequireEqStr(copy["Section"]["b"_sv].value(), "2"_sv);
  mRequireEqStr(copy["Section"]["c"_sv].value(), "3"_sv);
  mRequireEqStr(copy["Section"]["d"_sv].value(), "4"_sv);
  mRequireEqStr(copy["Section"]["wrold"_sv].value(), "test"_sv);
  mRequireEqStr(copy["Section"]["last"_sv].value(), "check \\"_sv);

  mRequire(copy["Section"]["last"_sv].value().data() !=
           ini["Section"]["last"_sv].value().data());
}
