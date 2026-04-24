#include "cc/inih.hpp"
#include "cc/error.hpp"

////////////////////////////////////////////////////////////////////////////////////
//  INIH SECTION ITERATOR
////////////////////////////////////////////////////////////////////////////////////

InihSectionIter::InihSectionIter(const SDict<InihKey, StrView>& sections)
    : data_(sections) {}

bool InihSectionIter::operator==(std::nullptr_t o) const {
  return it_ >= data_.size();  // is end?
}
bool InihSectionIter::operator!=(std::nullptr_t o) const {
  return not(*this == o);
}
StrView InihSectionIter::key() const {
  return data_.at(it_).key.str;
}
StrView InihSectionIter::value() const {
  return data_.at(it_).value;
}
InihSectionKV InihSectionIter::operator*() const {
  auto v = data_.at(it_);
  return InihSectionKV{.key = v.key.str, .value = v.value};
}
InihSectionIter& InihSectionIter::operator++() {
  ++it_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////
//  INIH SECTION
////////////////////////////////////////////////////////////////////////////////////

InihSection::InihSection(size_t initial_capacity) {
  data_.reserve(initial_capacity);
}

StrView InihSection::value(StrHash key) const {
  const StrView* view = data_.find(InihKey(key));
  if (view == nullptr) {
    throw Err("Key not found by hash");
  }
  return *view;
}

void InihSection::insert(StrView key, StrView value) {
  if (data_.size() == data_.capacity()) {
    data_.resize(data_.size() * 2);
  }
  data_.insert(InihKey{.hash = StrHash(key), .str = key}, value);
}

void InihSection::sort() {
  data_.sort();
}

////////////////////////////////////////////////////////////////////////////////////
//  INIH ITERATOR
////////////////////////////////////////////////////////////////////////////////////

InihIter::InihIter(const SDict<InihKey, InihSection>& sections) : data_(sections) {}

bool InihIter::operator==(std::nullptr_t o) const {
  return it_ >= data_.size();  // is end?
}
bool InihIter::operator!=(std::nullptr_t o) const {
  return not(*this == o);
}
StrView InihIter::name() const {
  return data_.at(it_).key.str;
}
const InihSection& InihIter::section() const {
  return data_.at(it_).value;
}
InihKV InihIter::operator*() const {
  auto v = data_.at(it_);
  return InihKV{.key = v.key.str, .value = v.value};
}
InihIter& InihIter::operator++() {
  ++it_;
  return *this;
}

////////////////////////////////////////////////////////////////////////////////////
//  INIH
////////////////////////////////////////////////////////////////////////////////////

Inih Inih::parse(Str data, u32 expect_global_section_count) {
  Inih inih;
  inih.data_ = move(data);

  if (inih.data_.empty()) {
    return inih;
  }

  inih.sections_.reserve(expect_global_section_count);
  size_t first_section = 0;

  if (inih.data_[0] != '[') {
    first_section = inih.data_.find("\n[");
  }

  StrView global_section_content = inih.data_.sub(0, first_section);
  parse_properties(global_section_content, inih.global_section_);

  StrView content = inih.data_.sub(first_section);

  while (true) {
    if (content.empty()) {
      break;
    }
    size_t start = content.find('[');
    size_t end   = start + 1;
    while (end < content.size() and content[end] != ']') {
      ++end;
    }
    if (end == content.size()) {
      break;
    }
    StrView header = content.sub(start + 1, end - start - 1);
    content        = content.sub(end + 1);  // not content does not include section header
    size_t  next_header     = content.find("\n[");
    StrView section_content = content.sub(0, next_header);
    if (next_header == StrView::npos) {
      content = StrView();
    } else {
      content = content.sub(next_header + 1);
    }
    InihSection section;
    parse_properties(section_content, section);
    if (inih.sections_.size() == inih.sections_.capacity()) {
      inih.sections_.resize(inih.sections_.size() * 2);
    }
    InihKey key = InihKey{.hash = StrHash(header), .str = header};
    inih.sections_.insert(key, move(section));
  }

  inih.sections_.sort();
  return inih;
}

const InihSection& Inih::global() const {
  return global_section_;
}

const InihSection& Inih::operator[](StrHash name) const {
  const InihSection* value = sections_.find(InihKey{.hash = name});
  if (value == nullptr) {
    throw Err("No section found");
  }
  return *value;
}

size_t Inih::section_count() const {
  return sections_.size();
}

InihKV Inih::operator[](size_t index) const {
  auto v = sections_.at(index);
  return InihKV{.key = v.key.str, .value = v.value};
}

void Inih::parse_properties(StrView section_content, InihSection& section) {
  size_t section_line_count = 1;
  for (size_t i = 0; i < section_content.size(); i++) {
    if (section_content[i] == '\n') {
      ++section_line_count;
    }
  }
  section = InihSection(section_line_count);

  while (true) {
    // section_content = section_content.trim_left();
    if (section_content.empty()) {
      break;
    }

    size_t  next_line = section_content.find('\n');
    StrView line      = section_content.sub(0, next_line);
    if (next_line == StrView::npos) {
      section_content = StrView();
    } else {
      section_content = section_content.sub(next_line + 1);
    }

    StrView          kv[2];
    ArrView<StrView> kv_split = line.split('=', kv);
    if (kv_split.size() != 2) {
      continue;
    }

    StrView key   = kv_split[0].trim();
    StrView value = kv_split[1].trim();
    section.insert(key, value);
  }
  section.sort();
}
