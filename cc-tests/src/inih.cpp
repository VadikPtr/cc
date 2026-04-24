#include "cc/test.hpp"
#include "cc/all.hpp"
#include "cc/inih.hpp"

mTestCase(inih_parse) {
  Inih ini = Inih::parse(R"(
global = test

[Section1]  
a = b
c = d

[Section]

  a = 1
b=2
c =3

d  =  4
arr = [1, 2, 3]
 wrold  = test
last = check

)"_s);

  for (auto [name, section] : ini) {
    mLogDebug("section: ", name);
    for (auto [key, value] : section) {
      mLogDebug("  prop ", key, ": ", value);
    }
  }

  mRequireEqStr(ini.global().value("global"_sh), "test"_sv);
  mRequireEqStr(ini["Section"_sh].value("a"_sh), "1"_sv);

  mRequire(ini["Section"_sh].parse<u64>("b"_sh) == u64(2));
  mRequire(ini["Section"_sh].parse_or<u64>("b"_sh, 4) == u64(2));
  mRequireEqStr(ini["Section"_sh].parse<Str>("c"_sh), "3"_sv);
  mRequireEqStr(ini["Section"_sh].parse<StrView>("d"_sh), "4"_sv);
  mRequireEqStr(ini["Section"_sh]["wrold"_sh], "test"_sv);
  mRequireEqStr(ini["Section"_sh]["last"_sh], "check"_sv);

  mRequireEqStr(ini["Section1"_sh].value("a"_sh), "b"_sv);

  int arr[3] = {};
  ini["Section"_sh].parse("arr"_sh, ArrView(arr));
  mRequire(arr[0] == 1);
  mRequire(arr[1] == 2);
  mRequire(arr[2] == 3);


  Inih small_ini = Inih::parse("[sec]\nkey=value"_s);
  mRequireEqStr(small_ini["sec"_sh]["key"_sh], "value"_sv);

  Inih space_fter_section = Inih::parse("[sec]  \nkey=value"_s);
  mRequireEqStr(space_fter_section["sec"_sh]["key"_sh], "value"_sv);

  Inih empty_ini           = Inih::parse("   "_s);
  Inih empty_multiline_ini = Inih::parse(" \n \n "_s);
}
