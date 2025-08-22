#pragma once
#include "cc/str.hpp"
#include "cc/str-view.hpp"
#include "cc/fmt.hpp"

typedef struct ini_t ini_t;

class Ini;
class IniSection;

class IniProp {
  friend IniSection;
  ini_t* ini_      = nullptr;
  int    section_  = 0;
  int    property_ = 0;
  IniProp(ini_t* ini, int section, int property);

 public:
  IniProp() = default;
  operator bool() const;
  explicit operator StrView() const;
  StrView  operator*() const;
  StrView  name() const;
  StrView  value() const;
  void     set_name(StrView name);
  void     set_value(StrView value);
  void     remove();

  template <typename T>
  T value() const {
    T result;
    parse_str(value(), result);
    return result;
  }

  template <typename T>
  void value(T& out) const {
    parse_str(value(), out);
  }

  template <typename T>
  T value_or(T def) const {
    T result;
    if (!StrParser<T>::try_parse(value(), result)) {
      return def;
    }
    return result;
  }
};

class IniSection {
  friend Ini;
  ini_t* ini_     = nullptr;
  int    section_ = 0;
  IniSection(ini_t* ini, int section);

 public:
  IniSection() = default;
  operator bool() const;
  IniProp operator[](StrView name) const;
  StrView name() const;
  void    set_name(StrView name);
  void    remove();

  class Iterator {
    ini_t* ini_               = nullptr;
    int    section_           = 0;
    int    property_          = 0;
    int    property_internal_ = 0;

   public:
    Iterator() = default;
    Iterator(ini_t* ini, int section, int property, int property_internal);
    IniProp   operator*() const;
    Iterator& operator++();
    Iterator& reset();
    bool      operator==(const Iterator& other) const;
    bool      operator!=(const Iterator& other) const;
  };

  Iterator        begin() const;
  static Iterator end();
};

class Ini {
  ini_t* ini_ = nullptr;

 public:
  Ini() = default;
  explicit Ini(ini_t* ini) : ini_(ini) {}
  ~Ini();

  Ini(const Ini& other);
  Ini& operator=(const Ini& other);

  Ini(Ini&& other) noexcept;
  Ini& operator=(Ini&& other) noexcept;

  static Ini parse(StrView str);
  static Ini create();

  bool       is_valid() const { return ini_ != nullptr; }
  void       destroy();
  Str        to_str() const;
  IniSection operator[](StrView name) const;
  IniSection operator[](size_t section_index) const;
  size_t     section_count() const;

  class Iterator {
    ini_t* ini_     = nullptr;
    int    section_ = 0;

   public:
    Iterator() = default;
    Iterator(ini_t* ini, int section);
    IniSection operator*() const;
    Iterator&  operator++();
    bool       operator==(const Iterator& other) const;
    bool       operator!=(const Iterator& other) const;
  };

  Iterator        begin() const;
  static Iterator end();
};
