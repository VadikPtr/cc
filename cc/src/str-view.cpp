#include "cc/str-view.hpp"
#include "cc/dict.hpp"
#include "cc/hash.hpp"
#include "cc/log.hpp"
#include "cc/str.hpp"
#include "cc/error.hpp"
#include <cctype>

namespace {
  // independent implementation of strnicmp, because it is not available on some platforms
  int str_compare_ci(const char* s1, const char* s2, size_t len) {
    while (len--) {
      int ic1 = tolower(*s1++);
      int ic2 = tolower(*s2++);
      if (ic1 != ic2) {
        return ic1 - ic2;
      }
    }
    return 0;
  }

  bool is_trimmable(char c) {
    return std::isspace(c) || c == '\n' || c == '\r' || c == '\t';
  }

#ifdef _DEBUG
  Dict<u64, Str> g_string_hashes;
#endif
}  // namespace

StrView::StrView(const char* str) {
  if (str != nullptr) {
    if (auto len = strlen(str)) {
      data_ = const_cast<char*>(str);
      size_ = len;
    }
  }
}

StrView::StrView(const char* str, size_t size) {
  if (size) {
    assert(str != nullptr);
    data_ = const_cast<char*>(str);
    size_ = size;
  }
}

bool StrView::operator==(StrView sv) const {
  if (size_ != sv.size()) {
    return false;
  }
  if (size_ == 0) {
    return true;
  }
  return strncmp(data_, sv.data(), size_) == 0;
}

void StrView::assign(StrView other) {
  if (size() != other.size() || empty()) {
    return;
  }
  memcpy(data_, other.data(), size());
}

bool StrView::starts_with(char c) const {
  if (empty()) {
    return false;
  }
  return data_[0] == c;
}

bool StrView::ends_with(char c) const {
  if (empty()) {
    return false;
  }
  return data_[size_ - 1] == c;
}

bool StrView::starts_with(StrView sv) const {
  if (sv.size() > size()) {
    return false;
  }
  if (empty() || sv.empty()) {
    return true;
  }
  return strncmp(data_, sv.data(), sv.size()) == 0;
}

bool StrView::ends_with(StrView sv) const {
  if (sv.size() > size()) {
    return false;
  }
  if (empty() || sv.empty()) {
    return true;
  }
  return strncmp(data_ + size_ - sv.size(), sv.data(), sv.size()) == 0;
}

ComparePos StrView::compare(StrView sv) const {
  auto min_size = mMin(size_, sv.size_);
  if (min_size == 0) {
    if (size_ < sv.size()) return ComparePos::Less;
    if (size_ > sv.size()) return ComparePos::Greater;
    return ComparePos::Equals;
  }
  auto v = strncmp(data_, sv.data(), min_size);
  if (v < 0) return ComparePos::Less;
  if (v > 0) return ComparePos::Greater;
  if (size_ == sv.size()) return ComparePos::Equals;
  if (size_ < sv.size()) return ComparePos::Less;
  return ComparePos::Greater;
}

ComparePos StrView::compare_ci(StrView sv) const {
  auto min_size = mMin(size_, sv.size_);
  if (min_size == 0) {
    if (size_ < sv.size()) return ComparePos::Less;
    if (size_ > sv.size()) return ComparePos::Greater;
    return ComparePos::Equals;
  }
  auto v = str_compare_ci(data_, sv.data(), min_size);
  if (v < 0) return ComparePos::Less;
  if (v > 0) return ComparePos::Greater;
  if (size_ == sv.size()) return ComparePos::Equals;
  if (size_ < sv.size()) return ComparePos::Less;
  return ComparePos::Greater;
}

u64 StrView::hash() const {
  return cc::hash_fnv64(data_, size_);
}

size_t StrView::find(char c) const {
  for (size_t i = 0; i < size_; i++) {
    if (data_[i] == c) {
      return i;
    }
  }
  return npos;
}

size_t StrView::find_last(char c) const {
  for (size_t i = size_ - 1; i != npos; --i) {
    if (data_[i] == c) {
      return i;
    }
  }
  return npos;
}

size_t StrView::find(StrView term) const {
  if (term.empty()) {
    return 0;
  }
  if (term.size_ > size_) {
    return npos;
  }
  size_t ssize = size_ - term.size_;
  for (size_t i = 0; i <= ssize; i++) {
    auto from_to = sub(i, term.size_);
    if (from_to == term) {
      return i;
    }
  }
  return npos;
}

size_t StrView::find_last(StrView term) const {
  if (term.empty()) {
    return 0;
  }
  if (term.size_ > size_) {
    return npos;
  }
  size_t ssize = size_ - term.size_;
  for (size_t i = ssize; i != npos; --i) {
    auto from_to = sub(i, term.size_);
    if (from_to == term) {
      return i;
    }
  }
  return npos;
}

StrView StrView::sub(size_t from, size_t count) const {
  if (empty() || !count || from > size_) {
    return {};
  }
  size_t max_count = size_ - from;
  if (count > max_count) {
    count = max_count;
  }
  return {data_ + from, count};
}

ArrView<StrView> StrView::split(char by, ArrView<StrView> out) const {
  if (empty() || out.empty()) {
    return {};
  }

  size_t pos, index = 0;
  auto   cur = *this;

  while ((pos = cur.find(by)) != npos) {
    auto append  = cur.sub(0, pos);
    out[index++] = append;
    if (index == out.size()) {
      return out;
    }
    cur = cur.sub(pos + 1);
  }

  if (index != out.size()) {
    out[index++] = cur;
  }

  return out.sub(0, index);
}

ArrView<StrView> StrView::split_se(char by, ArrView<StrView> out) const {
  if (empty() || out.empty()) {
    return {};
  }

  size_t pos, index = 0;
  auto   cur = *this;

  while ((pos = cur.find(by)) != npos) {
    auto append = cur.sub(0, pos);
    if (!append.empty()) {
      out[index++] = append;
      if (index == out.size()) {
        return out;
      }
    }
    cur = cur.sub(pos + 1);
  }

  if (index != out.size() && !cur.empty()) {
    out[index++] = cur;
  }

  return out.sub(0, index);
}

ArrView<StrView> StrView::split(StrView by, ArrView<StrView> out) const {
  if (empty() || out.empty()) {
    return {};
  }

  if (by.empty()) {
    out[0] = *this;
    return out.sub(0, 1);
  }

  size_t pos, index = 0;
  auto   cur = *this;

  while ((pos = cur.find(by)) != npos) {
    auto append  = cur.sub(0, pos);
    out[index++] = append;
    if (index == out.size()) {
      return out;
    }
    cur = cur.sub(pos + by.size());
  }

  if (index != out.size()) {
    out[index++] = cur;
  }

  return out.sub(0, index);
}

ArrView<StrView> StrView::split_se(StrView by, ArrView<StrView> out) const {
  if (empty() || out.empty()) {
    return {};
  }

  if (by.empty()) {
    out[0] = *this;
    return out.sub(0, 1);
  }

  size_t pos, index = 0;
  auto   cur = *this;

  while ((pos = cur.find(by)) != npos) {
    auto append = cur.sub(0, pos);
    if (!append.empty()) {
      out[index++] = append;
      if (index == out.size()) {
        return out;
      }
    }
    cur = cur.sub(pos + by.size());
  }

  if (index != out.size() && !cur.empty()) {
    out[index++] = cur;
  }

  return out.sub(0, index);
}


StrView StrView::trim_left() const {
  u64 begin = 0;
  for (size_t i = 0; i < size_; ++i) {
    if (!is_trimmable(data_[begin])) {
      break;
    }
    begin++;
  }
  return sub(begin);
}

StrView StrView::trim_right() const {
  if (!empty()) {
    u64 size = size_;
    for (size_t i = size_ - 1; i != npos; --i) {
      if (!is_trimmable(data_[i])) {
        return sub(0, size);
      }
      size--;
    }
  }
  return StrView();
}

StrView StrView::trim() const {
  return trim_left().trim_right();
}

StrView StrView::to_lower() {
  for (size_t i = 0; i < size_; i++) {
    data_[i] = tolower(data_[i]);
  }
  return *this;
}

StrView StrView::to_upper() {
  for (size_t i = 0; i < size_; i++) {
    data_[i] = toupper(data_[i]);
  }
  return *this;
}

bool StrView::try_to_c_str(char* buf, size_t buffer_size) const {
  if (buffer_size == 0 || buf == nullptr || size_ > buffer_size - 1) {
    return false;
  }
  if (size_ > 0) {
    memcpy(buf, data_, size_);
  }
  buf[size_] = 0;
  return true;
}

void StrView::to_c_str(char* buf, size_t buffer_size) const {
  if (!try_to_c_str(buf, buffer_size)) {
    throw Err("Not enough memory to convert string to c string"_s);
  }
}

StrHash::StrHash(StrView str) : hash_(str.hash()) {
#ifdef _DEBUG
  auto it = g_string_hashes.find(hash_);
  if (it != g_string_hashes.end() && it.value() != str) {
    mLogCrit("hash value duplicated: [", it.value(), "], [", str, "]: ", hash_);
  }
  g_string_hashes.insert(u64(hash_), Str(str));
#endif
}
