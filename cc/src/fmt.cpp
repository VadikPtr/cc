#include "cc/fmt.hpp"
#include "cc/dict.hpp"
#include "cc/str.hpp"

namespace {
  template <typename T, T max_value>
  constexpr bool parse_int(const char* str, size_t len, T& out)
    requires(std::is_integral_v<T>)
  {
    using UnsignedT          = std::make_unsigned_t<T>;
    constexpr bool is_signed = std::is_signed_v<T>;

    UnsignedT result   = 0;
    size_t    i        = 0;
    bool      negative = false;

    if (i < len && (str[i] == '-' || str[i] == '+')) {
      if (!is_signed && str[i] == '-') {
        return false;  // Can't parse negative into unsigned type
      }
      negative = (str[i] == '-');
      i++;
    }

    if (i >= len || str[i] < '0' || str[i] > '9') {
      return false;  // No digits
    }

    UnsignedT cutoff;

    if constexpr (is_signed) {
      cutoff = negative ? UnsignedT(max_value) + 1 : UnsignedT(max_value);
    } else {
      cutoff = max_value;
    }

    while (i < len && str[i] >= '0' && str[i] <= '9') {
      auto digit = static_cast<u8>(str[i] - '0');
      if (result > (cutoff - digit) / 10) {
        return false;  // Overflow
      }
      result = result * 10 + digit;
      i++;
    }

    if (i != len) {
      return false;
    }

    if (negative) {
      if constexpr (is_signed) {
        out = T(-result);
      }
    } else {
      out = T(result);
    }

    return true;
  }

  template <typename T>
  bool parse_float(const char* str, size_t len, T& out)
    requires(std::is_floating_point_v<T>)
  {
    size_t i        = 0;
    bool   negative = false;

    if (i < len && (str[i] == '-' || str[i] == '+')) {
      negative = str[i] == '-';
      i++;
    }

    if (i >= len || str[i] < '0' || str[i] > '9') {
      return false;
    }

    T result = 0;

    while (i < len && str[i] >= '0' && str[i] <= '9') {
      result = result * 10 + (str[i] - '0');
      i++;
    }

    if (i < len && str[i] == '.') {
      i++;
      T frac = 0;
      T base = T(0.1);
      while (i < len && str[i] >= '0' && str[i] <= '9') {
        frac += (str[i] - '0') * base;
        base *= T(0.1);
        i++;
      }
      result += frac;
    }

    if (i != len) {
      return false;
    }

    out = negative ? -result : result;
    return true;
  }

  template <typename T, size_t buffer_size>
  StrView to_string(T value, char (&buffer)[buffer_size])
    requires(std::is_integral_v<T>)
  {
    assert(buffer_size != 0);
    char* ptr = buffer + buffer_size;

    bool is_negative = value < 0;
    if (is_negative) {
      value = -value;
    }

    do {
      assert(ptr != buffer);
      *--ptr = value % 10 + '0';
      value /= 10;
    } while (value != 0);

    if (is_negative) {
      assert(ptr != buffer);
      *--ptr = '-';
    }

    size_t result_size = buffer_size - size_t(ptr - buffer);
    return {ptr, result_size};
  }

  template <typename T, size_t buffer_size>
  StrView to_string(T value, char (&buffer)[buffer_size], size_t trailing_count)
    requires(std::is_floating_point_v<T>)
  {
    assert(trailing_count + 2 < buffer_size);
    long long integer_part    = static_cast<long long>(value);
    T         fractional_part = std::abs(value - T(integer_part));
    bool      is_negative     = value < T(0);
    char*     trailing_ptr    = buffer + buffer_size - trailing_count;
    char*     ptr             = trailing_ptr;

    for (size_t i = 0; i < trailing_count; ++i) {
      fractional_part *= 10;
      int digit     = int(fractional_part);
      *trailing_ptr = char(digit + '0');
      ++trailing_ptr;
      fractional_part -= T(digit);
    }

    *--ptr = '.';

    if (integer_part == 0) {
      *--ptr = '0';
    } else {
      if (integer_part < 0) {
        integer_part = -integer_part;
      }
      while (integer_part > 0) {
        if (ptr == buffer + 1) {
          break;
        }
        *--ptr = '0' + integer_part % 10;
        integer_part /= 10;
      }
    }

    if (ptr != buffer && is_negative) {
      *--ptr = '-';
    }

    size_t result_size = buffer_size - size_t(ptr - buffer);
    return {ptr, result_size};
  }
}  // namespace


mFmtImpl(unsigned long long) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(unsigned long) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(bool) {
  if (v) {
    out.append("true");
  } else {
    out.append("false");
  }
}
mFmtImpl(s16) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(u16) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(ZeroPrefixU16) {
  char    buf[32];
  StrView r    = to_string(v.value, buf);
  size_t  size = r.size();
  char*   ptr  = r.data();
  while (size < v.zero_count) {
    assert(ptr != buf);
    *--ptr = '0';
    size++;
  }
  out.append(StrView(ptr, size));
}
mFmtImpl(s32) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(u32) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(s64) {
  char buf[32];
  out.append(to_string(v, buf));
}
mFmtImpl(f32) {
  char buf[32];
  out.append(to_string(v, buf, 4));
}
mFmtImpl(f64) {
  char buf[32];
  out.append(to_string(v, buf, 5));
}
mFmtImpl(Ptr) {
  char  buf[18] = {'0', 'x'};
  char* ptr     = buf + sizeof(buf);
  for (int i = 0; i < 16; ++i) {
    u8 digit = v.address >> (i * 4) & 0xF;
    *--ptr   = digit < 10 ? '0' + digit : 'A' + (digit - 10);
  }
  out.append(StrView(buf, sizeof(buf)));
}

mFmtImpl(HumanMemorySize) {
  static_assert(sizeof(size_t) == sizeof(u64));
  static struct {
    size_t      max;
    const char* name;
  } units[] = {
      {1ull << 10, "K"},  // 1024 Bytes = 1 KiB
      {1ull << 20, "M"},  // 1024 KiB = 1 MiB
      {1ull << 30, "G"},  // 1024 MiB = 1 GiB
  };

  if (v.size == 0) {
    out.append("0B");
    return;
  }
  for (const auto& unit : ArrView{units}) {
    if (v.size >= unit.max) {
      auto num = f64(v.size) / f64(unit.max);
      out.appendf("%.1f%s", num, unit.name);
      return;
    }
  }
  Fmt<size_t>::format(v.size, out);
  out.append('B');
}

mFmtImpl(StrView) {
  out.append(v);
}
mFmtImpl(Str) {
  out.append(v);
}
mFmtImpl(StrHash) {
  static_assert(sizeof(StrHash) == sizeof(u64));
  char  buf[18] = {'0', 'x'};
  char* ptr     = buf + sizeof(buf);
  for (int i = 0; i < 16; ++i) {
    u8 digit = v.hash() >> (i * 4) & 0xF;
    *--ptr   = digit < 10 ? '0' + digit : 'A' + (digit - 10);
  }
  out.append(StrView(buf, sizeof(buf)));
}
mFmtImpl(char) {
  out.append(v);
}

mStrParserImpl(unsigned long long) {
  return parse_int<unsigned long long, ULLONG_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(unsigned long) {
  return parse_int<unsigned long, ULONG_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(bool) {
  if (str == "true") {
    out = true;
    return true;
  } else if (str == "false") {
    out = false;
    return true;
  }
  return false;
}
mStrParserImpl(u16) {
  return parse_int<u16, UINT16_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(s16) {
  return parse_int<s16, INT16_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(u32) {
  return parse_int<u32, UINT32_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(s32) {
  return parse_int<s32, INT32_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(s64) {
  return parse_int<s64, INT64_MAX>(str.data(), str.size(), out);
}
mStrParserImpl(f32) {
  return parse_float(str.data(), str.size(), out);
}
mStrParserImpl(f64) {
  return parse_float(str.data(), str.size(), out);
}
mStrParserImpl(StrView) {
  out = str;
  return true;
}
mStrParserImpl(Str) {
  out = str;
  return true;
}
mStrParserImpl(StrWithWrap) {
  StrBuilder builder;
  while (!str.empty()) {
    auto del = str.find('\\');
    builder.append(str.sub(0, del));
    if (del == UINT64_MAX) {
      break;
    }
    str = str.sub(del + 1);
  }
  out = {builder.to_string()};
  return true;
}
