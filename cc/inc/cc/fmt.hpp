#pragma once
#include "cc/str.hpp"
#include "cc/error.hpp"
#include "cc/list.hpp"

template <typename T>
struct Fmt {
  static void format(const T&, StrBuilder&) { static_assert(false); }
};

#define mFmtDeclare(T)                         \
  template <>                                  \
  struct Fmt<T> {                              \
    static void format(const T&, StrBuilder&); \
  };

#define mFmtImpl(T) void Fmt<T>::format(const T& v, StrBuilder& out)

struct Ptr {
  u64 address;
  template <typename T>
  Ptr(const T* ptr) : address(u64(ptr)) {}
  Ptr(u64 address) : address(address) {}
};
struct HumanMemorySize {
  size_t size;
};
struct ZeroPrefixU16 {
  u16 zero_count;
  u16 value;
};

mFmtDeclare(unsigned long long);
mFmtDeclare(unsigned long);
mFmtDeclare(bool);
mFmtDeclare(ZeroPrefixU16);
mFmtDeclare(u8);
mFmtDeclare(u16);
mFmtDeclare(s16);
mFmtDeclare(u32);
mFmtDeclare(s32);
mFmtDeclare(s64);
mFmtDeclare(f32);
mFmtDeclare(f64);
mFmtDeclare(Ptr);
mFmtDeclare(HumanMemorySize);
mFmtDeclare(Str);
mFmtDeclare(StrView);
mFmtDeclare(StrHash);
mFmtDeclare(char);

template <size_t t>
struct Fmt<char[t]> {
  static void format(const char (&v)[t], StrBuilder& out) { out.append(StrView(v)); }
};

template <typename T>
struct Fmt<ArrView<T>> {
  static void format(const ArrView<T>& v, StrBuilder& out) {
    Fmt<StrView>::format("["_sv, out);
    for (size_t i = 0; i < v.size(); i++) {
      Fmt<T>::format(v[i], out);
      if (i != v.size() - 1) {
        Fmt<StrView>::format(", "_sv, out);
      }
    }
    Fmt<StrView>::format("]"_sv, out);
  }
};

template <typename T>
struct Fmt<List<T>> {
  static void format(const List<T>& list, StrBuilder& out) {
    Fmt<StrView>::format("["_sv, out);
    size_t i = 0;
    for (const auto& value : list) {
      Fmt<T>::format(value, out);
      if (i != list.size() - 1) {
        Fmt<StrView>::format(", "_sv, out);
      }
      ++i;
    }
    Fmt<StrView>::format("]"_sv, out);
  }
};

template <typename... Args>
void fmt(StrBuilder& result, const Args&... args) {
  (Fmt<Args>::format(args, result), ...);
}

template <typename... Args>
Str fmt(const Args&... args) {
  StrBuilder result;
  (Fmt<Args>::format(args, result), ...);
  return result.to_string();
}

template <typename T>
struct StrParser {
  static bool try_parse(StrView, T&) { static_assert(false); }
};

template <typename T>
void parse_str(StrView view, T& out) {
  if (!StrParser<T>::try_parse(view, out)) {
    throw Err(Str(fmt("Cannot parse string: ", view)));
  }
}

#define mStrParserDeclare(T)                    \
  template <>                                   \
  struct StrParser<T> {                         \
    static bool try_parse(StrView str, T& out); \
  };

#define mStrParserImpl(T) bool StrParser<T>::try_parse(StrView str, T& out)

struct StrWithWrap {
  Str str;
};

mStrParserDeclare(unsigned long long);
mStrParserDeclare(unsigned long);
mStrParserDeclare(bool);
mStrParserDeclare(u16);
mStrParserDeclare(s16);
mStrParserDeclare(u32);
mStrParserDeclare(s32);
mStrParserDeclare(s64);
mStrParserDeclare(f32);
mStrParserDeclare(f64);
mStrParserDeclare(StrView);
mStrParserDeclare(Str);
mStrParserDeclare(StrWithWrap);

template <typename T>
struct StrParser<ArrView<T>> {
  static bool try_parse(StrView str, ArrView<T>& out) {
    if (str.empty()) return false;
    if (str[0] != '[') return false;
    str = str.sub(1);

    for (size_t i = 0; i < out.size(); ++i) {
      if (str.empty()) {
        return false;
      }
      size_t pos = str.find(", ");
      if (pos == StrView::npos) {
        if (i + 1 != out.size()) {
          return false;
        }
        pos = str.size() - 1;
      }
      bool res = StrParser<T>::try_parse(str.sub(0, pos), out[i]);
      if (!res) {
        return false;
      }
      str = str.sub(i + 1 != out.size() ? pos + 2 : pos);
    }

    if (str.empty()) return false;
    if (str[0] != ']') return false;
    return true;
  }
};

template <typename T>
struct StrParser<List<T>> {
  static bool try_parse(StrView str, List<T>& out) {
    if (str.empty()) return false;
    if (str[0] != '[') return false;
    str = str.sub(1);

    for (;;) {
      if (str.empty()) {
        return false;
      }
      if (str[0] == ']') {
        return str.size() == 1;
      }
      size_t pos  = str.find(", ");
      size_t skip = 2;
      if (pos == StrView::npos) {
        pos  = str.size() - 1;
        skip = 0;
      }
      T val;
      bool res = StrParser<T>::try_parse(str.sub(0, pos), val);
      if (!res) {
        return false;
      }
      out.push_back(move(val));
      str = str.sub(pos + skip);
    }
  }
};

template <typename T, size_t N>
struct StrParser<T[N]> {
  static bool try_parse(StrView str, T (&out)[N]) {
    ArrView arr(out);
    return StrParser<ArrView<T>>::try_parse(str, arr);
  }
};
