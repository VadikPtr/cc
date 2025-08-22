#include "cc/test.hpp"
#include "cc/str.hpp"

#include "cc/fmt.hpp"

auto g_static_string_test = "StaticString";

mTestCase(str_default_ctor) {
  Str s;
  mRequire(s.empty());
  mRequire(s.size() == 0);
  mRequire(s.data() == nullptr);
}

mTestCase(str_size_ctor) {
  Str s(12);
  mRequire(!s.empty());
  mRequire(s.size() == 12);
  mRequire(s.data() != nullptr);
}

mTestCase(str_fill_ctor) {
  Str s(12, 'A');
  mRequire(s.size() == 12);
  mRequire(s.data() != nullptr);
  for (int i = 0; i < 12; ++i) {
    mRequire(s.data()[i] == 'A');
  }
}

mTestCase(str_cstr_ctor) {
  Str s("hello");
  mRequire(s.size() == 5);
  mRequireEqBytes(s.data(), "hello", 5);
  s = "test";
  mRequire(s.size() == 4);
  mRequireEqBytes(s.data(), "test", 4);
}

mTestCase(str_view_ctor) {
  StrView sv = "hello";
  Str     s(sv);
  mRequire(s.size() == 5);
  mRequireEqBytes(s.data(), "hello", 5);
  s = StrView{"test"};
  mRequire(s.size() == 4);
  mRequireEqBytes(s.data(), "test", 4);
  auto sv2 = StrView(s);
  mRequire(sv2.size() == s.size());
  mRequire(sv2.data() == s.data());
  s = sv2;
  mRequire(s.size() == 4);
  mRequireEqBytes(s.data(), "test", 4);
}

mTestCase(str_copy_assignment) {
  Str str("hello");
  Str copy;
  copy = str;
  mRequire(str.size() == copy.size());
  mRequire(str.size() == 5);
  mRequire(str.data() != nullptr);
  mRequire(copy.data() != nullptr);
  mRequire(copy.data() != str.data());
  mRequireEqBytes(str.data(), "hello", 5);
  mRequireEqBytes(copy.data(), "hello", 5);
}

mTestCase(str_move_ctor) {
  Str  s("hello");
  auto old_data = s.data();
  auto old_size = s.size();
  Str  s2(move(s));
  mRequire(s2.size() == old_size);
  mRequire(s2.data() == old_data);
}

mTestCase(str_move_assignment) {
  Str  s("hello");
  auto old_data = s.data();
  auto old_size = s.size();
  Str  s2;
  s2 = move(s);
  mRequire(s2.size() == old_size);
  mRequire(s2.data() == old_data);
}

mTestCase(str_resize_grow) {
  Str s(5, 'x');
  s.resize(10);
  mRequire(s.size() == 10);
  mRequire(s.data() != nullptr);
  memset(s.data() + 5, 'x', 5);
  mRequireEqBytes(s.data(), "xxxxxxxxxx", s.size());
}

mTestCase(str_resize_shrink) {
  Str s(10, 'y');
  s.resize(5);
  mRequire(s.size() == 5);
  mRequire(s.data() != nullptr);
  mRequireEqBytes(s.data(), "yyyyy", 5);
}

mTestCase(str_resize_zero) {
  Str s(10, 'y');
  s.resize(0);
  mRequire(s.size() == 0);
  mRequire(s.data() == nullptr);
  s.resize(3);
  mRequire(s.size() == 3);
  mRequire(s.data() != nullptr);
}

mTestCase(str_for_loop) {
  Str s(10, 'x');
  for (auto& c : s) {
    mRequire(c == 'x');
  }
  for (auto& c : s) {
    c = 'y';
  }
  const Str cs = s;
  for (const auto& c : cs) {
    mRequire(c == 'y');
  }
}

mTestCase(str_eq_op) {
  Str s("hello");
  mRequire(s == "hello");
  mRequire(s != "hell");
  mRequire(s != "helloo");
  mRequire(s == Str("hello"));
  mRequire(s != Str("hell"));
  mRequire(s != Str("helloo"));

  Str empty;
  mRequire(empty == "");
  mRequire(empty == Str(""));
  mRequire(empty == nullptr);
}

mTestCase(str_compare) {
  mRequire(StrView("VeryLongString").compare("Short") == ComparePos::Greater);
  mRequire(StrView("Hello").compare("Hello") == ComparePos::Equals);
  mRequire(StrView("HELLO").compare("hello") == ComparePos::Less);
  mRequire(StrView("World").compare("Earth") == ComparePos::Greater);
  mRequire(StrView("abcd").compare("abce") == ComparePos::Less);
  mRequire(StrView("a").compare("b") == ComparePos::Less);
  mRequire(StrView("A").compare("A") == ComparePos::Equals);
  mRequire(StrView("").compare("") == ComparePos::Equals);
  mRequire(StrView("").compare("abc") == ComparePos::Less);
  mRequire(StrView(nullptr).compare("") == ComparePos::Equals);
  mRequire(StrView(nullptr).compare("abc") == ComparePos::Less);
}

mTestCase(str_compare_ci) {
  mRequire(StrView("HellO wOrLd!").compare_ci("hello world!") == ComparePos::Equals);
  mRequire(StrView("HeLLoWoRLd").compare_ci("hellOWORLD") == ComparePos::Equals);
  mRequire(StrView("Hello1").compare_ci("hElLo") == ComparePos::Greater);
  mRequire(StrView("Hello").compare_ci("hElLo") == ComparePos::Equals);
  mRequire(StrView("HELLO").compare_ci("hello") == ComparePos::Equals);
  mRequire(StrView("Abcde").compare_ci("ABCDE") == ComparePos::Equals);
  mRequire(StrView("Apple").compare_ci("Orange") != ComparePos::Equals);
  mRequire(StrView("Test").compare_ci("") != ComparePos::Equals);
  mRequire(StrView("").compare_ci("") == ComparePos::Equals);
  mRequire(StrView(nullptr).compare_ci(nullptr) == ComparePos::Equals);
}

mTestCase(strview_sub) {
  StrView sv("0123456");
  mRequireEqStr(sv.sub(0), "0123456");
  mRequireEqStr(sv.sub(0, 0), "");
  mRequireEqStr(sv.sub(0, 1), "0");
  mRequireEqStr(sv.sub(0, 2), "01");
  mRequireEqStr(sv.sub(2, 0), "");
  mRequireEqStr(sv.sub(2, 1), "2");
  mRequireEqStr(sv.sub(2, 2), "23");
  mRequireEqStr(sv.sub(2), "23456");
  mRequireEqStr(sv.sub(6), "6");
  mRequireEqStr(sv.sub(6, 0), "");
  mRequireEqStr(sv.sub(6, 1), "6");
  mRequireEqStr(sv.sub(7, 10), "");
}

mTestCase(strview_find_char) {
  StrView sv("0123456");
  mRequire(sv.find('0') == 0);
  mRequire(sv.find('9') == UINT64_MAX);
  mRequire(sv.find('6') == 6);
}

mTestCase(strview_find_str) {
  StrView sv("0123456");
  mRequire(sv.find("0") == 0);
  mRequire(sv.find("0123456") == 0);
  mRequire(sv.find("01") == 0);
  mRequire(sv.find("012") == 0);
  mRequire(sv.find("12") == 1);
  mRequire(sv.find("123") == 1);
  mRequire(sv.find("123456") == 1);
  mRequire(sv.find("6") == 6);
  mRequire(StrView("112").sub(1).find('2') == 1);
  mRequire(StrView("112").sub(1).find('1') == 0);
  mRequire(StrView("01234501").find_last('0') == 6);
  mRequire(StrView("01234501").find_last('1') == 7);
  mRequire(StrView("121212").find_last("12") == 4);
  mRequire(StrView("121200").find_last("12") == 2);
}

mTestCase(strview_split) {
  StrView s_cont[8];

  auto s_view = StrView("1,2,3").split(',', s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView("").split(',', s_cont);
  mRequire(s_view.empty());

  s_view = StrView("abcd").split(',', s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abcd");

  s_view = StrView(",,").split(',', s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "");
  mRequireEqStr(s_view[1], "");
  mRequireEqStr(s_view[2], "");

  s_view = StrView("1,2,3,").split(',', s_cont);
  mRequire(s_view.size() == 4);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");
  mRequireEqStr(s_view[3], "");

  s_view = StrView("1,,2,3,").split(',', s_cont);
  mRequire(s_view.size() == 5);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "");
  mRequireEqStr(s_view[2], "2");
  mRequireEqStr(s_view[3], "3");
  mRequireEqStr(s_view[4], "");

  s_view = StrView(",abc").split(',', s_cont);
  mRequire(s_view.size() == 2);
  mRequireEqStr(s_view[0], "");
  mRequireEqStr(s_view[1], "abc");

  StrView s_small[1];
  s_view = StrView(",abc").split(',', s_small);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "");
}

mTestCase(strview_split_skip_empty) {
  StrView s_cont[8];

  auto s_view = StrView("1,2,3").split_se(',', s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView("").split_se(',', s_cont);
  mRequire(s_view.empty());

  s_view = StrView("abcd").split_se(',', s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abcd");

  s_view = StrView(",,").split_se(',', s_cont);
  mRequire(s_view.empty());

  s_view = StrView("1,2,3,").split_se(',', s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView("1,,2,3,").split_se(',', s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView(",abc").split_se(',', s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abc");

  StrView s_small[1];
  s_view = StrView(",abc").split_se(',', s_small);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abc");
}

mTestCase(strview_split_string) {
  StrView s_cont[8];

  auto s_view = StrView("1ab2ab3").split("ab", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView("ab1ab2").split("ab", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "");
  mRequireEqStr(s_view[1], "1");
  mRequireEqStr(s_view[2], "2");

  s_view = StrView("1ab2ab").split("ab", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "");

  s_view = StrView("1abab2").split("ab", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "");
  mRequireEqStr(s_view[2], "2");

  s_view = StrView("123").split("ab", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "123");

  s_view = StrView("ab").split("ab", s_cont);
  mRequire(s_view.size() == 2);
  mRequireEqStr(s_view[0], "");
  mRequireEqStr(s_view[1], "");

  s_view = StrView("").split("ab", s_cont);
  mRequire(s_view.empty());

  s_view = StrView("abc").split("", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abc");

  s_view = StrView("a,b,c").split(",", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "a");
  mRequireEqStr(s_view[1], "b");
  mRequireEqStr(s_view[2], "c");

  s_view = StrView("a").split("abc", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "a");

  StrView s_min[3];
  s_view = StrView("1ab2ab3ab4").split("ab", s_min);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");
}

mTestCase(strview_split_string_skip_empty) {
  StrView s_cont[8];

  auto s_view = StrView("1ab2ab3").split_se("ab", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");
  mRequireEqStr(s_view[2], "3");

  s_view = StrView("ab1ab2").split_se("ab", s_cont);
  mRequire(s_view.size() == 2);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");

  s_view = StrView("1ab2ab").split_se("ab", s_cont);
  mRequire(s_view.size() == 2);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");

  s_view = StrView("1abab2").split_se("ab", s_cont);
  mRequire(s_view.size() == 2);
  mRequireEqStr(s_view[0], "1");
  mRequireEqStr(s_view[1], "2");

  s_view = StrView("123").split_se("ab", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "123");

  s_view = StrView("ab").split_se("ab", s_cont);
  mRequire(s_view.empty());

  s_view = StrView("").split_se("ab", s_cont);
  mRequire(s_view.empty());

  s_view = StrView("abc").split_se("", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "abc");

  s_view = StrView("a,b,c").split_se(",", s_cont);
  mRequire(s_view.size() == 3);
  mRequireEqStr(s_view[0], "a");
  mRequireEqStr(s_view[1], "b");
  mRequireEqStr(s_view[2], "c");

  s_view = StrView("a").split_se("abc", s_cont);
  mRequire(s_view.size() == 1);
  mRequireEqStr(s_view[0], "a");
}

mTestCase(str_concat) {
  mRequireEqStr(Str("a") + Str("b"), Str("ab"));
  mRequireEqStr(Str("a") += Str("b"), Str("ab"));
  mRequireEqStr(Str("a") + Str(""), Str("a"));
  mRequireEqStr(Str("a") += Str(""), Str("a"));
  mRequireEqStr(Str("") + Str("b"), Str("b"));
  mRequireEqStr(Str("") += Str("b"), Str("b"));
  mRequireEqStr(Str("") + Str(""), Str(""));
  mRequireEqStr(Str("") += Str(""), Str(""));
  mRequireEqStr(Str::concat(Str(""), StrView("")), Str(""));
  mRequireEqStr(Str::concat(Str("a")), Str("a"));
  mRequireEqStr(Str::concat(Str("a"), StrView("b"), Str("c")), Str("abc"));
}

mTestCase(str_starts_ends) {
  mRequire(StrView("abc").ends_with("c"));
  mRequire(StrView("abc").ends_with('c'));
  mRequire(StrView("abc").starts_with("a"));
  mRequire(StrView("abc").starts_with('a'));

  mRequire(StrView("abc").ends_with(""));
  mRequire(StrView("abc").starts_with(""));
  mRequire(StrView("").ends_with(""));
  mRequire(StrView("").starts_with(""));

  mRequire(!StrView("abc").ends_with("abcd"));
  mRequire(!StrView("abc").starts_with("abcd"));
  mRequire(!StrView("").ends_with("abcd"));
  mRequire(!StrView("").starts_with("abcd"));
}
