#pragma once
#include "cc/sdict.hpp"
#include "cc/str-view.hpp"
#include "cc/str.hpp"
#include "cc/fmt.hpp"

struct InihKey {
  StrHash hash;
  StrView str;

  bool operator==(const InihKey& o) const { return hash == o.hash; }
  bool operator!=(const InihKey& o) const { return hash != o.hash; }
  bool operator<(const InihKey& o) const { return hash < o.hash; }
};

struct InihSectionKV {
  StrView key;
  StrView value;
};

// TODO: this is better SDict iterator, need to make single version
class InihSectionIter {
  size_t                         it_ = 0;
  const SDict<InihKey, StrView>& data_;

 public:
  explicit InihSectionIter(const SDict<InihKey, StrView>& sections);
  StrView key() const;
  StrView value() const;

  bool             operator==(std::nullptr_t o) const;
  bool             operator!=(std::nullptr_t o) const;
  InihSectionKV    operator*() const;
  InihSectionIter& operator++();
};

class InihSection {
  SDict<InihKey, StrView> data_;
  friend class Inih;

 public:
  InihSection() = default;
  explicit InihSection(size_t initial_capacity);

  StrView value(StrHash key) const;

  template <typename T>
  T parse(StrHash key) const {
    T result;
    parse_str(value(key), result);
    return result;
  }

  template <typename T>
  void parse(StrHash key, T& out) const {
    parse_str(value(key), out);
  }

  template <typename T>
  void parse(StrHash key, T&& out) const {
    parse_str(value(key), out);
  }

  template <typename T>
  T parse_or(StrHash key, T def) const {
    const StrView* view = data_.find(InihKey(key));
    if (view == nullptr) {
      return def;
    }
    T result;
    if (!StrParser<T>::try_parse(*view, result)) {
      return def;
    }
    return result;
  }

  InihSectionIter begin() const { return InihSectionIter(data_); }
  std::nullptr_t  end() const { return nullptr; }

 private:
  void insert(StrView key, StrView value);
  void sort();
};

struct InihKV {
  StrView            key;
  const InihSection& value;
};

class InihIter {
  size_t                             it_ = 0;
  const SDict<InihKey, InihSection>& data_;

 public:
  explicit InihIter(const SDict<InihKey, InihSection>& sections);
  StrView            name() const;
  const InihSection& section() const;

  bool      operator==(std::nullptr_t o) const;
  bool      operator!=(std::nullptr_t o) const;
  InihKV    operator*() const;
  InihIter& operator++();
};

class Inih {
  Str                         data_;
  InihSection                 global_section_;
  SDict<InihKey, InihSection> sections_;

 public:
  Inih() = default;

  Inih(const Inih&)            = delete;
  Inih& operator=(const Inih&) = delete;

  Inih(Inih&&) noexcept            = default;
  Inih& operator=(Inih&&) noexcept = default;

  void parse(Str data);

  InihIter           begin() const { return InihIter(sections_); }
  std::nullptr_t     end() const { return nullptr; }
  const InihSection& global() const;
  const InihSection& section(StrHash name) const;

 private:
  void parse_properties(StrView section_content, InihSection& section);
  void insert(StrView key, InihSection section);
};
