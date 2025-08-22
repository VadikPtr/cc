#include "cc/test.hpp"
#include "cc/fmt.hpp"

namespace {
  struct MyCustomType {
    int a;
    int b;
  };

  template <typename T>
  bool approx_equal(T a, T b, T epsilon = T(1e-6)) {
    T diff    = std::fabs(a - b);
    T max_abs = std::fmax(std::fabs(a), std::fabs(b));  // unit in the last place?
    return diff <= epsilon * max_abs;
  }
}  // namespace

template <>
struct Fmt<MyCustomType> {
  static void format(const MyCustomType& v, StrBuilder& out) {
    out.appendf("(%d, %d)", v.a, v.b);
  }
};

extern const char* g_static_string_test;

mTestCase(fmt_basic) {
  mRequireEqStr(fmt("Hello, world"), "Hello, world");
  mRequireEqStr(fmt(""), "");

  mRequireEqStr(fmt("int: ", 1234), "int: 1234");
  mRequireEqStr(fmt("int2: ", -1234), "int2: -1234");
  mRequireEqStr(fmt("int0: ", 0), "int0: 0");
  mRequireEqStr(fmt("int-max: ", INT32_MAX), "int-max: 2147483647");
  mRequireEqStr(fmt("ptr: ", Ptr{0x11223344'55667788}), "ptr: 0x1122334455667788");
  mRequireEqStr(fmt("float: ", 12.34f), "float: 12.3400");
  mRequireEqStr(fmt("float-: ", -12.34f), "float-: -12.3400");
  mRequireEqStr(fmt("float-0: ", -0.34f), "float-0: -0.3400");
  mRequireEqStr(fmt("double: ", 1.234567), "double: 1.23456");  // no rounding :(

  mRequireEqStr(fmt("str: ", Str{"Test String"}), "str: Test String");
  char buf[] = {'C', 's', 't', 'r', 'a', 'r', 'r', 0};
  mRequireEqStr(fmt("cstra: ", buf), "cstra: Cstrarr");
  mRequireEqStr(fmt("cstr: ", "C String"_sv), "cstr: C String");
  mRequireEqStr(fmt("cstrs: ", StrView{g_static_string_test}), "cstrs: StaticString");
  mRequireEqStr(fmt("str-view: ", StrView{"Test String"}), "str-view: Test String");

  mRequireEqStr(fmt("multi-value: ", 1, " ", 2u, " ", 3ull, " "), "multi-value: 1 2 3 ");
  mRequireEqStr(fmt("Custom: ", MyCustomType{1, 2}), "Custom: (1, 2)");

  int arr[] = {1, 2, 3, 4};
  mRequireEqStr(fmt("arr[int]: ", ArrView(arr)), "arr[int]: [1, 2, 3, 4]");

  MyCustomType arr2[] = {{1, 2}, {3, 4}};
  mRequireEqStr(fmt("arr[ct]: ", ArrView(arr2)), "arr[ct]: [(1, 2), (3, 4)]");

  char buf2[64] = {};
  sprintf(buf2, "%llu", ULLONG_MAX);
  mRequireEqStr(fmt(ULLONG_MAX), buf2);

  mRequireEqStr(fmt(ZeroPrefixU16{4, 0}), "0000");
  mRequireEqStr(fmt(ZeroPrefixU16{4, 1}), "0001");
  mRequireEqStr(fmt(ZeroPrefixU16{4, 10}), "0010");
  mRequireEqStr(fmt(ZeroPrefixU16{4, 123}), "0123");
  mRequireEqStr(fmt(ZeroPrefixU16{10, 123}), "0000000123");
  mRequireEqStr(fmt(ZeroPrefixU16{0, 123}), "123");
  mRequireEqStr(fmt(ZeroPrefixU16{1, 123}), "123");
  mRequireEqStr(fmt(ZeroPrefixU16{2, 123}), "123");
  mRequireEqStr(fmt(ZeroPrefixU16{3, 123}), "123");
}

mTestCase(parse_integer) {
  // --- ✅ Базовые случаи ---
  {
    s16 res = 0;
    mRequire(StrParser<s16>::try_parse("123", res));
    mRequire(res == 123);
  }
  {
    u32 res = 0;
    mRequire(StrParser<u32>::try_parse("0", res));
    mRequire(res == 0);
  }
  {
    s32 res = 0;
    mRequire(StrParser<s32>::try_parse("-98765", res));
    mRequire(res == -98765);
  }

  // --- ✅ Граничные значения ---
  {
    s16  res = 0;
    char buf[7];
    int  len = snprintf(buf, sizeof(buf), "%d", INT16_MAX);  // "32767"
    mRequire(StrParser<s16>::try_parse({buf, size_t(len)}, res));
    mRequire(res == INT16_MAX);
  }
  {
    s16  res = 0;
    char buf[8];
    int  len = snprintf(buf, sizeof(buf), "%d", INT16_MIN);  // "-32768"
    mRequire(StrParser<s16>::try_parse({buf, size_t(len)}, res));
    mRequire(res == INT16_MIN);
  }
  {
    u16  res = 0;
    char buf[6];
    int  len = snprintf(buf, sizeof(buf), "%u", USHRT_MAX);  // "65535"
    mRequire(StrParser<u16>::try_parse({buf, size_t(len)}, res));
    mRequire(res == USHRT_MAX);
  }
  {
    s32  res = 0;
    char buf[12];
    int  len = snprintf(buf, sizeof(buf), "%d", INT32_MAX);
    mRequire(StrParser<s32>::try_parse({buf, size_t(len)}, res));
    mRequire(res == INT32_MAX);
  }
  {
    s32  res = 0;
    char buf[13];
    int  len = snprintf(buf, sizeof(buf), "%d", INT32_MIN);
    mRequire(StrParser<s32>::try_parse({buf, size_t(len)}, res));
    mRequire(res == INT32_MIN);
  }

  // --- ❌ Переполнения ---
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("32768", res));  // > INT16_MAX
  }
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("-32769", res));  // < INT16_MIN
  }
  {
    u16 res = 0;
    mRequire(!StrParser<u16>::try_parse("65536", res));  // > UINT16_MAX
  }

  // --- ❌ Неверные форматы ---
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("", res));  // пустая строка
  }
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("abc", res));
  }
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("12a3", res));
  }
  {
    u16 res = 0;
    mRequire(!StrParser<u16>::try_parse("-1", res));
  }
  {
    u32 res = 0;
    mRequire(!StrParser<u32>::try_parse("+", res));  // только знак — ошибка
  }

  // --- ✅ Знак "+" ---
  {
    s32 res = 0;
    mRequire(StrParser<s32>::try_parse("+42", res));
    mRequire(res == 42);
  }
  {
    u64 res = 0;
    mRequire(StrParser<u64>::try_parse("+123456789", res));
    mRequire(res == 123456789);
  }

  // --- ❌ Лишние символы после числа ---
  {
    s16 res = 0;
    mRequire(!StrParser<s16>::try_parse("123abc", res));
  }
}

mTestCase(parse_float) {
  // --- ✅ Базовые случаи ---
  {
    f32 res = 0;
    mRequire(StrParser<f32>::try_parse("3.14", res));
    mRequire(approx_equal(res, 3.14f));
  }
  {
    f64 res = 0;
    mRequire(StrParser<f64>::try_parse("0.0", res));
    mRequire(approx_equal(res, 0.0));
  }
  {
    f32 res = 0;
    mRequire(StrParser<f32>::try_parse("123", res));
    mRequire(approx_equal(res, 123.0f));
  }
  {
    f64 res = 0;
    mRequire(StrParser<f64>::try_parse("-42.5", res));
    mRequire(approx_equal(res, -42.5));
  }

  // --- ✅ Знак "+" ---
  {
    f32 res = 0;
    mRequire(StrParser<f32>::try_parse("+1.5", res));
    mRequire(approx_equal(res, 1.5f));
  }

  // --- ✅ Целая часть без дробной ---
  {
    f64 res = 0;
    mRequire(StrParser<f64>::try_parse("-1000", res));
    mRequire(approx_equal(res, -1000.0));
  }
  {
    f64 res = 0;
    mRequire(StrParser<f64>::try_parse("3.", res));
    mRequire(approx_equal<f64>(res, 3.0));
  }

  // --- ✅ Только дробная часть после точки ---
  {
    f32 res = 0;
    mRequire(StrParser<f32>::try_parse("0.125", res));
    mRequire(approx_equal(res, 0.125f));
  }

  // --- ❌ Пустая строка ---
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse("", res));
  }

  // --- ❌ Только знак ---
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse("-", res));
  }

  // --- ❌ Точка без цифр ---
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse(".", res));
  }

  // --- ❌ Нецифровые символы ---
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse("12a.3", res));
  }
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse("3.14abc", res));
  }

  // --- ❌ Две точки ---
  {
    f64 res = 0;
    mRequire(!StrParser<f64>::try_parse("1.2.3", res));
  }

  // --- ✅ Граничные значения (около FLT_MAX) ---
  {
    auto buf = "340282346638528859811704183484516925440.0";
    f32  res = 0;
    mRequire(StrParser<f32>::try_parse(buf, res));
    mRequire(approx_equal(res, 340282346638528859811704183484516925440.0f));
  }

  {
    auto buf = "340282346638528859811704183484516925440.0";
    f64  res = 0;
    mRequire(StrParser<f64>::try_parse(buf, res));
    mRequire(approx_equal(res, 340282346638528859811704183484516925440.0));
  }

  // --- ❌ Формат e ---
  {
    f32 res = 0;
    mRequire(!StrParser<f32>::try_parse("1e40", res));
  }
}

mTestCase(parse_array_int) {
  int     v[3];
  ArrView out(v);
  mRequire(StrParser<ArrView<int>>::try_parse("[1, 2, 3]", out));
  mRequire(v[0] == 1);
  mRequire(v[1] == 2);
  mRequire(v[2] == 3);

  mRequire(!StrParser<ArrView<int>>::try_parse("[1, 2]", out));
  mRequire(!StrParser<ArrView<int>>::try_parse("[1]", out));
  mRequire(!StrParser<ArrView<int>>::try_parse("[]", out));

  ArrView out_0(v, 0);
  mRequire(StrParser<ArrView<int>>::try_parse("[]", out_0));
  mRequire(!StrParser<ArrView<int>>::try_parse("[1]", out_0));

  ArrView out_1(v, 1);
  mRequire(!StrParser<ArrView<int>>::try_parse("[]", out_1));
  mRequire(StrParser<ArrView<int>>::try_parse("[3]", out_1));
  mRequire(v[0] == 3);
  mRequire(!StrParser<ArrView<int>>::try_parse("[3, 4]", out_1));
}

mTestCase(parse_array_string) {
  StrView values[2];
  ArrView out(values);
  parse_str("[val1, val2]", out);
  mRequireEqStr(values[0], "val1");
  mRequireEqStr(values[1], "val2");
}
