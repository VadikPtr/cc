#pragma once
#include "cc/common.hpp"
#include "cc/str.hpp"
#include "cc/fmt.hpp"
#include "cc/arr.hpp"

enum class FsType {
  NoExists,
  File,
  Directory,
};
template <>
struct Fmt<FsType> {
  static void format(const FsType& v, StrBuilder& out);
};

enum class FsDirMode {
  Default,
  Recursive,
};
template <>
struct Fmt<FsDirMode> {
  static void format(const FsDirMode& v, StrBuilder& out);
};

struct IFileVisitor {
  virtual ~IFileVisitor() = default;

  virtual bool visit(const class Path& path, FsType type) {
    (void)path;
    (void)type;
    return true;
  }
  virtual bool visit_dir_end(const Path& path) {
    (void)path;
    return true;
  }
};

class Path {
  Str data_;

 public:
  Path() = default;
  Path(Str str) : data_(move(str)) {}
  Path(StrView str) : data_(str) {}

  bool    empty() const { return data_.empty(); }
  bool    has_parent() const;
  Path    parent() const;
  Path    normalized() const;
  Path&   normalize();
  size_t  components_count() const;
  size_t  get_components(ArrView<StrView> out) const;
  StrView name() const;
  StrView name_without_ext() const;
  StrView ext() const;                      // includes .
  Path    with_ext(StrView new_ext) const;  // replaces extension
  Path    relative_to(const Path& base) const;
  Path    absolute() const;
  Path    try_absolute() const;  // empty if fails
  FsType  type() const;
  size_t  file_size() const;
  bool    try_create_dir(FsDirMode mode = FsDirMode::Default) const;
  bool    try_remove_dir(FsDirMode mode = FsDirMode::Default) const;
  bool    try_remove_file() const;
  bool    try_visit_dir(IFileVisitor& visitor, FsDirMode mode = FsDirMode::Default) const;
  void    create_dir(FsDirMode mode = FsDirMode::Default) const;
  void    remove_dir(FsDirMode mode = FsDirMode::Default) const;
  void    remove_file() const;
  void    visit_dir(IFileVisitor& visitor, FsDirMode mode = FsDirMode::Default) const;
  Arr<u8> read_bytes() const;

  const StrView& view() const { return data_; }
  u64            hash() const { return data_.hash(); }
  ComparePos     compare(StrView sv) const { return data_.compare(sv); }
  ComparePos     compare_ci(StrView sv) const { return data_.compare_ci(sv); }

  static Path join(StrView a, StrView b);
  static Path join(StrView a, StrView b, StrView c);
  static Path join(ArrView<StrView> views);
  static Path try_to_exe();  // empty if fails
  static Path try_to_cwd();  // empty if fails
  static Path to_exe();
  static Path to_cwd();

  explicit operator Str() const { return data_; }
  operator StrView() const { return data_; }
  operator bool() const { return !data_.empty(); }

  bool operator==(StrView other) const { return data_ == other; }
  bool operator!=(StrView other) const { return data_ != other; }
};

template <>
struct Fmt<Path> {
  static void format(const Path& v, StrBuilder& out);
};

inline Path operator""_p(const char* cstr, size_t size) {
  return {StrView{cstr, size}};
}

Path operator/(const Path& a, const Path& b);
Path operator/(const Path& a, StrView b);
Path operator/(StrView a, const Path& b);

class File {
  FILE* file_ = nullptr;

 public:
  File() = default;
  File(const Path& path, const char* mode);
  ~File() noexcept;
  File(const File&)            = delete;
  File& operator=(const File&) = delete;
  File(File&& other) noexcept;
  File& operator=(File&& other) noexcept;

  void open(const Path& path, const char* mode);
  bool try_open(const Path& path, const char* mode);
  bool is_valid() const { return file_ != nullptr; }
  void close();

  void read_bytes(ArrView<u8> out) const;
  bool try_read_bytes(ArrView<u8> out) const;
  void write_bytes(ArrView<u8> data) const;
  bool try_write_bytes(ArrView<u8> data) const;
};
