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

  mRequire(ini.global());
  mRequireEqStr(ini.global()["global"].value(), "test"_sv);

  mRequire(!ini["Unknown Section"]);
  mRequire(!ini["Unknown Section"]["Unknown prop"]);
  mRequire(ini["none"]["unknown"].value_or(2) == 2);
  mRequireEqStr(ini["Section"]["a"].value(), "1"_sv);
  mRequire(ini["Section"]["b"].value<u64>() == u64(2));
  mRequire(ini["Section"]["b"].value_or<u64>(4) == u64(2));
  mRequireEqStr(ini["Section"]["c"].value<Str>(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"].value<StrView>(), "4"_sv);
  mRequireEqStr(ini["Section"]["hello"].value(), "wrold \\\nhmmm"_sv);
  mRequireEqStr(ini["Section"]["hello"].value<StrWithWrap>().str, "wrold \nhmmm"_sv);
  mRequireEqStr(ini["Section"]["wrold"].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"].value(), "check \\"_sv);

  int arr[3] = {};
  ini["Section"]["arr"].value(arr);
  mRequire(arr[0] == 1);
  mRequire(arr[1] == 2);
  mRequire(arr[2] == 3);

  ini["Section"]["hello"].remove();
  mRequire(!ini["Section"]["hello"]);
  mRequireEqStr(ini["Section"]["a"].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"].value(), "check \\"_sv);

  mRequire(ini["Section to remove"]);
  ini["Section to remove"].remove();
  mRequire(!ini["Section to remove"]);
  mRequireEqStr(ini["Section"]["a"].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"].value(), "check \\"_sv);

  Ini copy;
  copy = ini;
  mRequireEqStr(ini["Section"]["a"].value(), "1"_sv);
  mRequireEqStr(ini["Section"]["b"].value(), "2"_sv);
  mRequireEqStr(ini["Section"]["c"].value(), "3"_sv);
  mRequireEqStr(ini["Section"]["d"].value(), "4"_sv);
  mRequireEqStr(ini["Section"]["wrold"].value(), "test"_sv);
  mRequireEqStr(ini["Section"]["last"].value(), "check \\"_sv);

  mRequireEqStr(copy["Section"]["a"].value(), "1"_sv);
  mRequireEqStr(copy["Section"]["b"].value(), "2"_sv);
  mRequireEqStr(copy["Section"]["c"].value(), "3"_sv);
  mRequireEqStr(copy["Section"]["d"].value(), "4"_sv);
  mRequireEqStr(copy["Section"]["wrold"].value(), "test"_sv);
  mRequireEqStr(copy["Section"]["last"].value(), "check \\"_sv);

  mRequire(copy["Section"]["last"].value().data() !=
           ini["Section"]["last"].value().data());
}
