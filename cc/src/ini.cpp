#include "cc/ini.hpp"

#include "cc/str.hpp"

// This file contains portions of ini.h library by Mattias Gustavsson.
/*
MIT License

Copyright (c) 2015 Mattias Gustavsson

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

struct ini_internal_section_t {
  char   name[32];
  char*  name_large;
  size_t name_size;
};

struct ini_internal_property_t {
  int    section;
  char   name[32];
  char*  name_large;
  size_t name_size;
  char   value[64];
  char*  value_large;
  size_t value_size;
};

struct ini_t {
  ini_internal_section_t*  sections;
  int                      section_capacity;
  int                      section_count;
  ini_internal_property_t* properties;
  int                      property_capacity;
  int                      property_count;
};

#define INI_GLOBAL_SECTION (0)
#define INI_NOT_FOUND (-1)
#define INITIAL_CAPACITY (16)

#define INI_STRLEN(s) (strlen(s))
#define INI_MEMCPY(dst, src, cnt) memcpy(dst, src, cnt)
#ifdef _WIN32
  #define INI_STRNICMP(s1, s2, cnt) (_strnicmp(s1, s2, size_t(cnt)))
  #define INI_STRDUP(s) (_strdup(s))
#else
  #define INI_STRNICMP(s1, s2, cnt) (strncasecmp(s1, s2, size_t(cnt)))
  #define INI_STRDUP(s) (strdup(s))
#endif

namespace {
  ini_t* ini_create();
  ini_t* ini_load(char const* data, size_t size);

  Str  ini_save(const ini_t* ini);
  void ini_destroy(ini_t* ini);

  int     ini_section_count(const ini_t* ini);
  StrView ini_section_name(const ini_t* ini, int section);

  // int     ini_property_count(const ini_t* ini, int section);
  StrView ini_property_name(const ini_t* ini, int section, int property);
  StrView ini_property_value(const ini_t* ini, int section, int property);

  int ini_find_section(const ini_t* ini, const char* name, size_t name_length);
  int ini_find_property(const ini_t* ini, int section, const char* name,
                        size_t name_length);

  int  ini_section_add(ini_t* ini, const char* name, size_t length);
  void ini_property_add(ini_t* ini, int section, const char* name, size_t name_length,
                        char const* value, size_t value_length);
  void ini_section_remove(ini_t* ini, int section);
  void ini_property_remove(ini_t* ini, int section, int property);

  void ini_section_name_set(ini_t* ini, int section, const char* name, size_t length);
  void ini_property_name_set(ini_t* ini, int section, int property, const char* name,
                             size_t length);
  void ini_property_value_set(ini_t* ini, int section, int property, char const* value,
                              size_t length);

  int ini_internal_property_index(const ini_t* ini, int section, int property) {
    if (ini && section >= 0 && section < ini->section_count) {
      int p = 0;
      for (int i = 0; i < ini->property_count; ++i) {
        if (ini->properties[i].section == section) {
          if (p == property) {
            return i;
          }
          ++p;
        }
      }
    }
    return INI_NOT_FOUND;
  }

  ini_t* ini_create() {
    auto ini = (ini_t*)malloc(sizeof(ini_t));
    ini->sections =
        (ini_internal_section_t*)malloc(INITIAL_CAPACITY * sizeof(ini->sections[0]));
    ini->section_capacity       = INITIAL_CAPACITY;
    ini->section_count          = 1; /* global section */
    ini->sections[0].name[0]    = '\0';
    ini->sections[0].name_size  = 0;
    ini->sections[0].name_large = nullptr;
    ini->properties =
        (ini_internal_property_t*)malloc(INITIAL_CAPACITY * sizeof(ini->properties[0]));
    ini->property_capacity = INITIAL_CAPACITY;
    ini->property_count    = 0;
    return ini;
  }

  ini_t* ini_load(char const* data, size_t size) {
    ini_t* ini = ini_create();

    if (!data || !size) {
      return ini;
    }

    char const* ptr = data;
    char const* start;
    int         section = 0;

    while (ptr != data + size) {
      /* trim leading whitespace */
      while (ptr != data + size && *ptr <= ' ') {
        ++ptr;
      }

      /* done? */
      if (ptr == data + size) {
        break;
      }

      /* comment */
      else if (*ptr == ';') {
        while (ptr != data + size && *ptr != '\n') {
          ++ptr;
        }
      }

      /* section */
      else if (*ptr == '[') {
        ++ptr;
        start = ptr;
        while (ptr != data + size && *ptr != ']' && *ptr != '\n') {
          ++ptr;
        }

        if (*ptr == ']') {
          section = ini_section_add(ini, start, (size_t)(ptr - start));
          ++ptr;
        }
      }

      /* property */
      else {
        start = ptr;
        while (ptr != data + size && *ptr != '=' && *ptr != '\n') {
          ++ptr;
        }

        if (*ptr == '=') {
          int l = (int)(ptr - start);
          while (l > 0 && start[l - 1] == ' ') {
            l--;
          }
          ++ptr;
          while (ptr != data + size &&
                 (*(ptr - 1) == '\\' || (*ptr <= ' ' && *ptr != '\n'))) {
            ptr++;
          }
          char const* start2 = ptr;
          while (ptr != data + size && (*(ptr - 1) == '\\' || *ptr != '\n')) {
            ++ptr;
          }
          while (*--ptr <= ' ') {
            (void)ptr;
          }
          ptr++;
          ini_property_add(ini, section, start, size_t(l), start2,
                           (size_t)(ptr - start2));
        }
      }
    }

    return ini;
  }

  Str ini_save(const ini_t* ini) {
    if (!ini) {
      return {};
    }

    Str    data(256);
    size_t pos = 0;

#define mAppendChar(c)            \
  if (pos == data.size()) {       \
    data.resize(data.size() * 2); \
  }                               \
  data[pos++] = c;

    for (int s = 0; s < ini->section_count; ++s) {
      char* n = ini->sections[s].name_large ? ini->sections[s].name_large
                                            : ini->sections[s].name;
      int   l = (int)ini->sections[s].name_size;
      if (l > 0) {
        mAppendChar('[');
        for (int i = 0; i < l; ++i) {
          mAppendChar(n[i]);
        }
        mAppendChar(']');
        mAppendChar('\n');
        ++pos;
      }

      for (int p = 0; p < ini->property_count; ++p) {
        if (ini->properties[p].section == s) {
          n = ini->properties[p].name_large ? ini->properties[p].name_large
                                            : ini->properties[p].name;
          l = (int)ini->properties[p].name_size;
          for (int i = 0; i < l; ++i) {
            mAppendChar(n[i]);
          }
          mAppendChar(' ');
          mAppendChar('=');
          mAppendChar(' ');
          n = ini->properties[p].value_large ? ini->properties[p].value_large
                                             : ini->properties[p].value;
          l = (int)ini->properties[p].value_size;
          for (int i = 0; i < l; ++i) {
            mAppendChar(n[i]);
          }
          mAppendChar('\n');
        }
      }

      if (pos > 0) {
        mAppendChar('\n');
      }
    }

    data.resize(pos);
    return data;
  }

  void ini_destroy(ini_t* ini) {
    if (!ini) {
      return;
    }
    for (int i = 0; i < ini->property_count; ++i) {
      free(ini->properties[i].value_large);
      free(ini->properties[i].name_large);
    }
    for (int i = 0; i < ini->section_count; ++i) {
      free(ini->sections[i].name_large);
    }
    free(ini->properties);
    free(ini->sections);
    free(ini);
  }

  int ini_section_count(const ini_t* ini) {
    if (ini) {
      return ini->section_count;
    }
    return 0;
  }

  StrView ini_section_name(const ini_t* ini, int section) {
    if (!ini || section < 0 || section >= ini->section_count) {
      return {};
    }
    const char* ptr  = ini->sections[section].name_large
                           ? ini->sections[section].name_large
                           : ini->sections[section].name;
    size_t      size = ini->sections[section].name_size;
    return {ptr, size};
  }

  // int ini_property_count(const ini_t* ini, int section) {
  //   if (!ini) {
  //     return 0;
  //   }
  //   int count = 0;
  //   for (int i = 0; i < ini->property_count; ++i) {
  //     if (ini->properties[i].section == section) {
  //       ++count;
  //     }
  //   }
  //   return count;
  // }

  StrView ini_property_name(const ini_t* ini, int section, int property) {
    if (ini && section >= 0 && section < ini->section_count) {
      int p = ini_internal_property_index(ini, section, property);
      if (p != INI_NOT_FOUND) {
        const char* ptr  = ini->properties[p].name_large ? ini->properties[p].name_large
                                                         : ini->properties[p].name;
        size_t      size = ini->properties[p].name_size;
        return {ptr, size};
      }
    }
    return {};
  }

  StrView ini_property_value(const ini_t* ini, int section, int property) {
    if (ini && section >= 0 && section < ini->section_count) {
      int p = ini_internal_property_index(ini, section, property);
      if (p != INI_NOT_FOUND) {
        const char* ptr  = ini->properties[p].value_large ? ini->properties[p].value_large
                                                          : ini->properties[p].value;
        size_t      size = ini->properties[p].value_size;
        return {ptr, size};
      }
    }
    return {};
  }

  int ini_find_section(const ini_t* ini, const char* name, size_t name_length) {
    if (ini && name) {
      for (int i = 0; i < ini->section_count; ++i) {
        if (ini->sections[i].name_size != name_length) {
          continue;
        }
        char const* const other = ini->sections[i].name_large
                                      ? ini->sections[i].name_large
                                      : ini->sections[i].name;
        if (INI_STRNICMP(name, other, name_length) == 0) {
          return i;
        }
      }
    }
    return INI_NOT_FOUND;
  }

  int ini_find_property(const ini_t* ini, int section, const char* name,
                        size_t name_length) {
    if (!ini || !name || section < 0 || section >= ini->section_count) {
      return INI_NOT_FOUND;
    }
    int c = 0;
    for (int i = 0; i < ini->property_count; ++i) {
      if (ini->properties[i].section == section) {
        if (ini->properties[i].name_size == name_length) {
          char const* const other = ini->properties[i].name_large
                                        ? ini->properties[i].name_large
                                        : ini->properties[i].name;
          if (INI_STRNICMP(name, other, name_length) == 0) {
            return c;
          }
        }
        ++c;
      }
    }
    return INI_NOT_FOUND;
  }

  int ini_section_add(ini_t* ini, const char* name, size_t length) {
    if (!ini || !name) {
      return INI_NOT_FOUND;
    }

    if (ini->section_count >= ini->section_capacity) {
      ini->section_capacity *= 2;
      auto new_sections = (ini_internal_section_t*)malloc(size_t(ini->section_capacity) *
                                                          sizeof(ini->sections[0]));
      INI_MEMCPY(new_sections, ini->sections,
                 (size_t)ini->section_count * sizeof(ini->sections[0]));
      free(ini->sections);
      ini->sections = new_sections;
    }

    ini->sections[ini->section_count].name_large = nullptr;
    ini->sections[ini->section_count].name_size  = length;

    if (length) {
      if (length > sizeof(ini->sections[0].name)) {
        ini->sections[ini->section_count].name_large = (char*)malloc(length);
        memcpy(ini->sections[ini->section_count].name_large, name, length);
      } else {
        memcpy(ini->sections[ini->section_count].name, name, length);
      }
    }

    return ini->section_count++;
  }

  void ini_property_add(ini_t* ini, int section, const char* name, size_t name_length,
                        char const* value, size_t value_length) {
    if (!ini || !name || section < 0 || section >= ini->section_count) {
      return;
    }

    if (ini->property_count >= ini->property_capacity) {
      ini->property_capacity *= 2;
      auto* new_properties = (ini_internal_property_t*)malloc(
          (size_t)ini->property_capacity * sizeof(ini->properties[0]));
      memcpy(new_properties, ini->properties,
             (size_t)ini->property_count * sizeof(ini->properties[0]));
      free(ini->properties);
      ini->properties = new_properties;
    }

    ini->properties[ini->property_count].section     = section;
    ini->properties[ini->property_count].name_large  = nullptr;
    ini->properties[ini->property_count].name_size   = name_length;
    ini->properties[ini->property_count].value_large = nullptr;
    ini->properties[ini->property_count].value_size  = value_length;

    if (name_length) {
      if (name_length > sizeof(ini->properties[0].name)) {
        ini->properties[ini->property_count].name_large = (char*)malloc(name_length);
        memcpy(ini->properties[ini->property_count].name_large, name, name_length);
      } else {
        memcpy(ini->properties[ini->property_count].name, name, name_length);
      }
    }

    if (value_length) {
      if (value_length > sizeof(ini->properties[0].value)) {
        ini->properties[ini->property_count].value_large = (char*)malloc(value_length);
        memcpy(ini->properties[ini->property_count].value_large, value, value_length);
      } else {
        memcpy(ini->properties[ini->property_count].value, value, value_length);
      }
    }

    ++ini->property_count;
  }

  void ini_section_remove(ini_t* ini, int section) {
    if (!ini || section < 0 || section >= ini->section_count) {
      return;
    }

    free(ini->sections[section].name_large);

    for (int p = ini->property_count - 1; p >= 0; --p) {
      if (ini->properties[p].section == section) {
        free(ini->properties[p].value_large);
        free(ini->properties[p].name_large);
        ini->properties[p] = ini->properties[--ini->property_count];
      }
    }

    ini->sections[section] = ini->sections[--ini->section_count];

    for (int p = 0; p < ini->property_count; ++p) {
      if (ini->properties[p].section == ini->section_count) {
        ini->properties[p].section = section;
      }
    }
  }

  void ini_property_remove(ini_t* ini, int section, int property) {
    if (!ini || section < 0 || section >= ini->section_count) {
      return;
    }
    int p = ini_internal_property_index(ini, section, property);
    if (p == INI_NOT_FOUND) {
      return;
    }
    free(ini->properties[p].value_large);
    free(ini->properties[p].name_large);
    if (int properties_left = ini->property_count - p - 1; properties_left > 0) {
      memmove(ini->properties + p, ini->properties + p + 1,
              sizeof(ini->properties[0]) * size_t(properties_left));
    }
    --ini->property_count;
  }

  void ini_section_name_set(ini_t* ini, int section, const char* name, size_t length) {
    if (!ini || !name || section < 0 || section >= ini->section_count) {
      return;
    }

    free(ini->sections[section].name_large);
    ini->sections[section].name_large = nullptr;
    ini->sections[section].name_size  = length;

    if (length > sizeof(ini->sections[0].name)) {
      ini->sections[section].name_large = (char*)malloc(length);
      memcpy(ini->sections[section].name_large, name, length);
    } else {
      memcpy(ini->sections[section].name, name, length);
    }
  }

  void ini_property_name_set(ini_t* ini, int section, int property, const char* name,
                             size_t length) {
    if (!ini || !name || section < 0 || section >= ini->section_count) {
      return;
    }

    int p = ini_internal_property_index(ini, section, property);
    if (p == INI_NOT_FOUND) {
      return;
    }

    free(ini->properties[p].name_large);
    ini->properties[ini->property_count].name_large = nullptr;
    ini->properties[ini->property_count].name_size  = length;

    if (length) {
      if (length > sizeof(ini->properties[0].name)) {
        ini->properties[p].name_large = (char*)malloc(length);
        memcpy(ini->properties[p].name_large, name, length);
      } else {
        memcpy(ini->properties[p].name, name, length);
      }
    }
  }

  void ini_property_value_set(ini_t* ini, int section, int property, char const* value,
                              size_t length) {
    if (!ini || !value || section < 0 || section >= ini->section_count) {
      return;
    }
    int p = ini_internal_property_index(ini, section, property);
    if (p == INI_NOT_FOUND) {
      return;
    }

    free(ini->properties[p].value_large);
    ini->properties[ini->property_count].value_large = nullptr;
    ini->properties[ini->property_count].value_size  = length;

    if (length) {
      if (length > sizeof(ini->properties[0].value)) {
        ini->properties[p].value_large = (char*)malloc(length);
        memcpy(ini->properties[p].value_large = (char*)malloc(length), value, length);
      } else {
        memcpy(ini->properties[p].value, value, length);
      }
    }
  }

  ini_t* ini_clone(ini_t* base) {
    if (base == nullptr) {
      return nullptr;
    }

    auto ini = (ini_t*)malloc(sizeof(ini_t));

    ini->section_capacity  = base->section_capacity;
    ini->section_count     = base->section_count;
    ini->property_capacity = base->property_capacity;
    ini->property_count    = base->property_count;

    ini->sections = (ini_internal_section_t*)malloc(size_t(base->section_capacity) *
                                                    sizeof(ini->sections[0]));
    memcpy(ini->sections, base->sections,
           size_t(base->section_count) * sizeof(ini->sections[0]));
    for (int p = 0; p < base->section_count; ++p) {
      auto& section = ini->sections[p];
      if (section.name_large) {
        section.name_large = INI_STRDUP(section.name_large);
      }
    }

    ini->properties = (ini_internal_property_t*)malloc(size_t(ini->property_capacity) *
                                                       sizeof(ini->properties[0]));
    memcpy(ini->properties, base->properties,
           sizeof(ini->properties[0]) * size_t(ini->property_count));
    for (int p = 0; p < base->property_count; ++p) {
      auto& property = base->properties[p];
      if (property.name_large) {
        property.name_large = INI_STRDUP(property.name_large);
      }
      if (property.value_large) {
        property.value_large = INI_STRDUP(property.value_large);
      }
    }

    return ini;
  }

}  // namespace


IniProp::IniProp(ini_t* ini, int section, int property)
    : ini_(ini), section_(section), property_(property) {}

IniProp::operator bool() const {
  return ini_ != nullptr;
}

IniProp::operator StrView() const {
  return ini_property_value(ini_, section_, property_);
}
StrView IniProp::operator*() const {
  return ini_property_value(ini_, section_, property_);
}
StrView IniProp::name() const {
  return ini_property_name(ini_, section_, property_);
}
StrView IniProp::value() const {
  return ini_property_value(ini_, section_, property_);
}

void IniProp::set_name(StrView name) {
  ini_property_name_set(ini_, section_, property_, name.data(), name.size());
}

void IniProp::set_value(StrView value) {
  ini_property_value_set(ini_, section_, property_, value.data(), value.size());
}

void IniProp::remove() {
  ini_property_remove(ini_, section_, property_);
  ini_      = nullptr;
  section_  = 0;
  property_ = 0;
}

IniSection::IniSection(ini_t* ini, int section) : ini_(ini), section_(section) {}

IniSection::operator bool() const {
  return ini_ != nullptr;
}

IniProp IniSection::operator[](StrView name) const {
  int property = ini_find_property(ini_, section_, name.data(), name.size());
  if (property == INI_NOT_FOUND) {
    return {};
  }
  return {ini_, section_, property};
}
StrView IniSection::name() const {
  return ini_section_name(ini_, section_);
}

void IniSection::set_name(StrView name) {
  ini_section_name_set(ini_, section_, name.data(), name.size());
}

void IniSection::remove() {
  ini_section_remove(ini_, section_);
  ini_     = nullptr;
  section_ = 0;
}

IniSection::Iterator::Iterator(ini_t* ini, int section, int property,
                               int property_internal)
    : ini_(ini),
      section_(section),
      property_(property),
      property_internal_(property_internal) {}

IniProp IniSection::Iterator::operator*() const {
  if (ini_ == nullptr) {
    return {};
  }
  return {ini_, section_, property_};
}

IniSection::Iterator& IniSection::Iterator::operator++() {
  for (int i = property_internal_ + 1; i < ini_->property_count; ++i) {
    if (ini_->properties[i].section == section_) {
      property_++;
      property_internal_ = i;
      return *this;
    }
  }
  return reset();
}

IniSection::Iterator& IniSection::Iterator::reset() {
  ini_               = nullptr;
  section_           = 0;
  property_          = 0;
  property_internal_ = 0;
  return *this;
}

bool IniSection::Iterator::operator==(const Iterator& other) const {
  return ini_ == other.ini_ && section_ == other.section_ && property_ == other.property_;
}

bool IniSection::Iterator::operator!=(const Iterator& other) const {
  return !(*this == other);
}

IniSection::Iterator IniSection::begin() const {
  for (int i = 0; i < ini_->property_count; ++i) {
    if (ini_->properties[i].section == section_) {
      return {ini_, section_, 0, i};
    }
  }
  return {};
}

IniSection::Iterator IniSection::end() {
  return {};
}

Ini::~Ini() {
  destroy();
}

Ini::Ini(const Ini& other) {
  ini_ = ini_clone(other.ini_);
}

Ini& Ini::operator=(const Ini& other) {
  if (this != &other) {
    destroy();
    ini_ = ini_clone(other.ini_);
  }
  return *this;
}

Ini::Ini(Ini&& other) noexcept {
  swap(ini_, other.ini_);
}

Ini& Ini::operator=(Ini&& other) noexcept {
  if (this != &other) {
    swap(ini_, other.ini_);
    other.destroy();
  }
  return *this;
}

void Ini::destroy() {
  ini_destroy(ini_);
  ini_ = nullptr;
}

Ini Ini::parse(StrView str) {
  return Ini{ini_load(str.data(), str.size())};
}

Ini Ini::create() {
  return Ini{ini_create()};
}

Str Ini::to_str() const {
  return ini_save(ini_);
}

IniSection Ini::operator[](StrView name) const {
  int section = ini_find_section(ini_, name.data(), name.size());
  if (section == INI_NOT_FOUND) {
    return {};
  }
  return {ini_, section};
}

IniSection Ini::operator[](size_t section_index) const {
  return {ini_, (int)section_index};
}

size_t Ini::section_count() const {
  return (size_t)ini_section_count(ini_);
}

Ini::Iterator::Iterator(ini_t* ini, int section) : ini_(ini), section_(section) {}

IniSection Ini::Iterator::operator*() const {
  return {ini_, section_};
}

Ini::Iterator& Ini::Iterator::operator++() {
  ++section_;
  if (section_ >= ini_section_count(ini_)) {
    ini_     = nullptr;
    section_ = 0;
  }
  return *this;
}

bool Ini::Iterator::operator==(const Iterator& other) const {
  return ini_ == other.ini_ && section_ == other.section_;
}

bool Ini::Iterator::operator!=(const Iterator& other) const {
  return !(*this == other);
}

Ini::Iterator Ini::begin() const {
  if (ini_ && ini_->section_count) {
    return {ini_, 0};
  }
  return {};
}

Ini::Iterator Ini::end() {
  return {};
}
