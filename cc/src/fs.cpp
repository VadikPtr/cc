#include "cc/fs.hpp"
#include "cc/fmt.hpp"
#include "cc/error.hpp"

#ifdef _WIN32
  #include <windows.h>
  #include <shellapi.h>
  #include <direct.h>
#else
  #include <unistd.h>
  #include <sys/stat.h>
  #include <dirent.h>
#endif

#ifdef __APPLE__
  #include <mach-o/dyld.h>
#endif

namespace {
  struct OsPath {
    char   cstr[512] = {};
    size_t len       = 0;

    OsPath() = default;

    OsPath(const char* str) { strcpy(cstr, str); }

    OsPath(const Path& path) {
      auto view = path.view();
      if (view.empty()) {
        throw Err(Str("Path is empty and cannot be used to do OS things"));
      }
      if (view.size() >= sizeof(cstr) - 3) {
        throw Err(fmt("Path ", view, " is too large to fit in OsPath"));
      }

      memcpy(cstr, view.data(), view.size());
      len = view.size();

#ifdef _WIN32
      for (size_t i = 0; i < view.size(); i++) {
        if (cstr[i] == '/') {
          cstr[i] = '\\';
        }
      }
#endif
    }

    operator Path() const {
      Path result(cstr);
      result.normalize();
      return result;
    }
  };
}  // namespace


void Fmt<FsType>::format(const FsType& v, StrBuilder& out) {
  switch (v) {
    case FsType::NoExists:
      return out.append("NoExists");
    case FsType::File:
      return out.append("File");
    case FsType::Directory:
      return out.append("Directory");
    default:
      out.append("<unknown>");
  }
}

void Fmt<FsDirMode>::format(const FsDirMode& v, StrBuilder& out) {
  switch (v) {
    case FsDirMode::Default:
      return out.append("Default");
    case FsDirMode::Recursive:
      return out.append("Recursive");
    default:
      return out.append("<unknown>");
  }
}

bool Path::has_parent() const {
  size_t count = 0;
  for (char c : data_) {
    if (c == '/' || c == '\\') {
      count++;
    }
  }
  return count >= 2;
}

Path Path::parent() const {
  if (data_.empty()) {
    return *this;
  }
  size_t slash = UINT64_MAX;
  for (size_t index = data_.size() - 1; index != UINT64_MAX; --index) {
    if (data_[index] == '/' || data_[index] == '\\') {
      slash = index;
      break;
    }
  }
  if (slash != UINT64_MAX) {
    return {data_.sub(0, slash)};
  }
  return {};
}

Path Path::normalized() const {
  Path result = *this;
  result.normalize();
  return result;
}

Path& Path::normalize() {
  size_t size = 0;
  char   prev = 0;
  for (char c : data_) {
    if (c == '/' || c == '\\') {
      if (prev != '/') {
        prev = data_[size++] = '/';
      }
    } else {
      prev = data_[size++] = c;
    }
  }
  if (size > 1 && prev == '/') {
    size--;
  }
  data_.resize(size);
  return *this;
}

size_t Path::components_count() const {
  size_t count = 0;
  size_t i     = 0;
  while (i < data_.size()) {
    while (i < data_.size() && data_[i] == '/') {
      ++i;
    }
    if (i < data_.size()) {
      ++count;
      while (i < data_.size() && data_[i] != '/') {
        ++i;
      }
    }
  }
  return count;
}

size_t Path::get_components(ArrView<StrView> out) const {
  // unsafe

  size_t      index = 0;
  const char* start = data_.data();
  const char* end   = data_.data() + data_.size();
  const char* curr  = start;

  while (curr < end) {
    while (curr < end && *curr == '/') {
      ++curr;
    }
    const char* component_start = curr;
    while (curr < end && *curr != '/') {
      ++curr;
    }
    if (component_start < curr) {
      out[index++] = {component_start, (size_t)(curr - component_start)};
    }
  }

  return index;
}

StrView Path::name() const {
  size_t idx = data_.find_last('/');
  if (idx == UINT64_MAX) {
    return view();
  }
  return data_.sub(idx + 1);
}

StrView Path::name_without_ext() const {
  auto   n   = name();
  size_t idx = n.find('.');
  if (idx == UINT64_MAX) {
    return n;
  }
  return n.sub(0, idx);
}

StrView Path::ext() const {
  auto   n   = name();
  size_t idx = n.find('.');
  if (idx == UINT64_MAX) {
    return n;
  }
  return n.sub(idx);
}

Path Path::with_ext(StrView new_ext) const {
  size_t slash = data_.find_last('/');
  if (slash == UINT64_MAX) {
    slash = 0;
  }
  auto   from_slash = data_.sub(slash);
  size_t dot        = from_slash.find('.');
  if (dot == UINT64_MAX) {
    dot = from_slash.size();
  }
  auto without_ext = data_.sub(0, slash + dot);
  return {Str::concat(without_ext, new_ext)};
}

Path Path::relative_to(const Path& base) const {
  constexpr size_t max_components = 48;

  size_t cur_count = components_count();
  if (cur_count > max_components) {
    throw Err(fmt("Too many path components: ", *this));
  }
  StrView cur_components[max_components];
  cur_count = get_components(cur_components);

  size_t base_count = base.components_count();
  if (base_count > max_components) {
    throw Err(fmt("Too many path components: ", base));
  }
  StrView base_components[max_components];
  base_count = base.get_components(base_components);

  size_t common = 0;
  while (common < base_count &&  //
         common < cur_count &&   //
         base_components[common] == cur_components[common]) {
    common++;
  }

  size_t  res_count = 0;
  StrView res_components[max_components];

  for (size_t i = common; i < base_count; ++i) {
    res_components[res_count++] = "..";
  }

  for (size_t i = common; i < cur_count; ++i) {
    res_components[res_count++] = cur_components[i];
  }

  return join(ArrView(res_components, res_count));
}

Path Path::absolute() const {
  if (auto res = try_absolute(); res.empty()) {
    throw Err(fmt("Cannot get absolute path for ", *this));
  } else {
    return res;
  }
}

Path Path::try_absolute() const {
  OsPath p(*this);
  OsPath out;
#ifdef _WIN32
  if (GetFullPathNameA(p.cstr, sizeof(out.cstr), out.cstr, nullptr) == 0) {
    out.cstr[0] = 0;
  }
#else
  if (realpath(p.cstr, out.cstr) == nullptr) {
    out.cstr[0] = 0;
  }
#endif
  return out;
}

Path Path::find_dir_up(StrView name) const {
  for (Path cur = *this;;) {
    if (auto d = cur / name; d.type() == FsType::Directory) {
      return d;
    }
    if (!cur.has_parent()) {
      throw Err(fmt("cannot find dir: ", StrView(name)));
    }
    cur = cur.parent();
  }
}

FsType Path::type() const {
  OsPath p(*this);
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad{};
  if (GetFileAttributesExA(p.cstr, GetFileExInfoStandard, &fad) == 0 ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_READONLY ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
    return FsType::NoExists;
  }
  if (fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    return FsType::Directory;
  }
  return FsType::File;
#else
  struct stat st;
  int32_t     result = ::stat(p.cstr, &st);
  if (0 != result) {
    return FsType::NoExists;
  }
  if (0 != (st.st_mode & S_IFREG)) {
    return FsType::File;
  }
  if (0 != (st.st_mode & S_IFDIR)) {
    return FsType::Directory;
  }
  return FsType::NoExists;
#endif
}

size_t Path::file_size() const {
  OsPath p(*this);
#ifdef _WIN32
  WIN32_FILE_ATTRIBUTE_DATA fad{};
  if (GetFileAttributesExA(p.cstr, GetFileExInfoStandard, &fad) == 0 ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_READONLY ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_DEVICE ||
      fad.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
    return 0;
  }
  LARGE_INTEGER size;
  size.HighPart = (LONG)fad.nFileSizeHigh;
  size.LowPart  = fad.nFileSizeLow;
  return size_t(size.QuadPart);
#else
  struct stat st;
  if (s32 result = ::stat(p.cstr, &st); result != 0 || (st.st_mode & S_IFREG) == 0) {
    return 0;
  }
  return size_t(st.st_size);
#endif
}

static bool platform_mkdir(const char* path, [[maybe_unused]] FsDirMode mode) {
#ifdef _WIN32
  return _mkdir(path) == 0;
#else
  if (mode == FsDirMode::Recursive) {
    struct stat st;
    if (stat(path, &st) == 0) {
      return true;  // already exists
    }

    OsPath parent(path);
    if (char* last_slash = strrchr(parent.cstr, '/')) {
      *last_slash = '\0';
      if (strlen(parent.cstr) > 0) {
        if (!platform_mkdir(parent.cstr, mode)) {
          return false;
        }
      }
    }
  }

  return mkdir(path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
#endif
}

bool Path::try_create_dir(FsDirMode mode) const {
  OsPath p(*this);
  return platform_mkdir(p.cstr, mode);
}

bool Path::try_remove_dir(FsDirMode mode) const {
  OsPath p(*this);
#ifdef _WIN32
  if (mode == FsDirMode::Recursive) {
    if (p.cstr[p.len - 1] != '\\') {
      p.cstr[p.len++] = '\\';
    }
    SHFILEOPSTRUCTA file_op{};
    file_op.wFunc  = FO_DELETE;
    file_op.pFrom  = p.cstr;
    file_op.fFlags = FOF_NO_UI | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;
    s32 result     = SHFileOperationA(&file_op);
    return result == 0 && !file_op.fAnyOperationsAborted;
  }

  return RemoveDirectoryA(p.cstr) != 0;
#else
  if (mode == FsDirMode::Recursive) {
    struct RemoveVisitor final : IFileVisitor {
      bool visit(const Path& path, FsType type) override {
        if (type == FsType::File) {
          return path.try_remove_file();
        }
        return true;
      }
      bool visit_dir_end(const class Path& path) override {
        return path.try_remove_dir(FsDirMode::Default);
      }
    } remove_visitor;

    return try_visit_dir(remove_visitor, FsDirMode::Recursive);
  }
  return rmdir(p.cstr) == 0;
#endif
}

bool Path::try_remove_file() const {
  OsPath p(*this);
#ifdef _WIN32
  return DeleteFileA(p.cstr) != 0;
#else
  return unlink(p.cstr) == 0;
#endif
}

bool Path::try_visit_dir(IFileVisitor& visitor, FsDirMode mode) const {
  OsPath p(*this);

#ifdef _WIN32
  p.cstr[p.len++] = '\\';
  p.cstr[p.len++] = '*';

  {
    WIN32_FIND_DATAA fd;
    HANDLE           h_find = FindFirstFileA(p.cstr, &fd);
    if (h_find == INVALID_HANDLE_VALUE) {
      return false;
    }
    FinalCleanup<BOOL, HANDLE, FindClose> cleanup_handle{h_find};

    do {
      if (fd.dwFileAttributes == INVALID_FILE_ATTRIBUTES ||
          fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN ||
          fd.dwFileAttributes & FILE_ATTRIBUTE_SYSTEM ||
          fd.dwFileAttributes & FILE_ATTRIBUTE_READONLY ||
          fd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) {
        continue;
      }

      const char* name = fd.cFileName;
      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        continue;
      }

      FsType type = fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ? FsType::Directory
                                                                   : FsType::File;

      auto inner_path = *this / StrView(name);
      if (!visitor.visit(inner_path, type)) {
        return false;
      }

      if (mode == FsDirMode::Recursive && type == FsType::Directory) {
        if (!inner_path.try_visit_dir(visitor, mode)) {
          return false;
        }
      }
    } while (FindNextFileA(h_find, &fd));
  }

  return visitor.visit_dir_end(*this);

#else

  DIR* dir = opendir(p.cstr);
  if (!dir) {
    return false;
  }

  {
    FinalCleanup<int, DIR*, closedir> cleanup_handle{dir};

    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
      const char* name = entry->d_name;
      if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
        continue;
      }

      auto inner_path = *this / name;
      auto type       = inner_path.type();
      if (type == FsType::NoExists) {
        continue;
      }

      if (!visitor.visit(inner_path, type)) {
        return false;
      }

      if (mode == FsDirMode::Recursive && type == FsType::Directory) {
        if (!inner_path.try_visit_dir(visitor, mode)) {
          return false;
        }
      }
    }
  }

  return visitor.visit_dir_end(*this);

#endif
}

void Path::create_dir(FsDirMode mode) const {
  if (!try_create_dir(mode)) {
    throw Err(fmt("Cannot create directory ", *this));
  }
}

void Path::remove_dir(FsDirMode mode) const {
  if (!try_remove_dir(mode)) {
    throw Err(fmt("Cannot remove directory ", *this));
  }
}

void Path::remove_file() const {
  if (!try_remove_file()) {
    throw Err(fmt("Cannot remove file ", *this));
  }
}

void Path::visit_dir(IFileVisitor& visitor, FsDirMode mode) const {
  if (!try_visit_dir(visitor, mode)) {
    throw Err(fmt("Cannot visit directory ", *this));
  }
}

Arr<u8> Path::read_bytes() const {
  size_t  sz = file_size();
  Arr<u8> res(sz);
  File(*this, "rb").read_bytes(res);
  return res;
}

Str Path::read_ctext() const {
  size_t sz = file_size();
  Str    res(sz + 1);
  File(*this, "rb").read_bytes(ArrView<u8>((u8*)res.data(), sz));
  res[sz] = 0;
  return res;
}

Str Path::read_text() const {
  size_t sz = file_size();
  Str    res(sz);
  File(*this, "rb").read_bytes(ArrView<u8>((u8*)res.data(), sz));
  return res;
}

Path Path::join(StrView a, StrView b) {
  if (a.empty()) {
    return b;
  }
  Path res{Str::concat(a, StrView{"/"}, b)};
  return res.normalized();
}

Path Path::join(StrView a, StrView b, StrView c) {
  if (a.empty()) {
    return join(b, c);
  }
  Path res{Str::concat(a, StrView{"/"}, b, StrView{"/"}, c)};
  return res.normalized();
}

Path Path::join(ArrView<StrView> views) {
  size_t count = 0;
  for (auto v : views) {
    count += v.size() + 1;
  }
  Str    str(count);
  size_t cur = 0;
  for (auto v : views) {
    str.sub(cur, v.size()).assign(v);
    cur += v.size();
    str[cur++] = '/';
  }
  return Path(move(str)).normalize();
}

Path Path::try_to_exe() {
  // TODO: handle errors in all cases
  OsPath p;
#if defined(_WIN32)
  GetModuleFileName(nullptr, p.cstr, sizeof(p.cstr));
#elif defined(__APPLE__)
  u32 size = (u32)sizeof(p.cstr);
  _NSGetExecutablePath(p.cstr, &size);
#else  // linux
  char proc_path[32] = {};
  snprintf(proc_path, sizeof(proc_path), "/proc/%d/exe", getpid());
  ssize_t bytes = readlink(proc_path, p.cstr, sizeof(p.cstr));
  p.cstr[bytes] = 0;
#endif
  return p;
}

Path Path::try_to_cwd() {
  // TODO: handle errors in all cases
  OsPath p;
#ifdef _WIN32
  _getcwd(p.cstr, sizeof(p.cstr));
#else
  getcwd(p.cstr, sizeof(p.cstr));
#endif
  return p;
}

Path Path::to_exe() {
  if (auto res = try_to_exe(); res.empty()) {
    throw Err(Str("Cannot get executable path"));
  } else {
    return res;
  }
}

Path Path::to_cwd() {
  if (auto res = try_to_cwd(); res.empty()) {
    throw Err(Str("Cannot get current working directory path"));
  } else {
    return res;
  }
}

void Fmt<Path>::format(const Path& v, StrBuilder& out) {
  out.append(v.view());
}

Path operator/(const Path& a, const Path& b) {
  return Path::join(a, b);
}

Path operator/(const Path& a, StrView b) {
  return Path::join(a, b);
}

Path operator/(StrView a, const Path& b) {
  return Path::join(a, b);
}

File::File(const Path& path, const char* mode) {
  open(path, mode);
}

File::~File() noexcept {
  close();
}

File::File(File&& other) noexcept {
  swap(file_, other.file_);
}

File& File::operator=(File&& other) noexcept {
  if (this != &other) {
    swap(file_, other.file_);
    other.close();
  }
  return *this;
}

void File::open(const Path& path, const char* mode) {
  if (!try_open(path, mode)) {
    throw Err(fmt("Cannot open file ", path, " (mode: ", StrView(mode), ")"));
  }
}

bool File::try_open(const Path& path, const char* mode) {
  close();
  OsPath p(path);
  file_ = fopen(p.cstr, mode);
  return bool(file_);
}

void File::close() {
  if (file_) {
    fclose(file_);
    file_ = nullptr;
  }
}

void File::read_bytes(ArrView<u8> out) const {
  if (!try_read_bytes(out)) {
    throw Err(Str("Cannot read file"));
  }
}

bool File::try_read_bytes(ArrView<u8> out) const {
  if (out.empty()) {
    return true;
  }

  size_t left_to_read = out.size();
  size_t read         = 0;

  do {
    auto current_read = fread(out.data() + read, 1, left_to_read, file_);

    if (feof(file_)) {
      read += current_read;
      break;
    }

    if (ferror(file_)) {
      // mVyLog( "error read file" ); // TODO
      return false;
    }

    left_to_read -= current_read;
    read += current_read;
  } while (read != out.size());

  // assert(read == out.size());
  return read == out.size();
}

void File::write_bytes(ArrView<u8> data) const {
  if (!try_write_bytes(data)) {
    throw Err(Str("Cannot write file"));
  }
}

bool File::try_write_bytes(ArrView<u8> data) const {
  if (data.empty()) {
    return true;
  }

  size_t left_to_write = data.size();
  size_t write         = 0;

  do {
    auto current_write = fwrite(data.data() + write, 1, left_to_write, file_);

    if (ferror(file_)) {
      // mVyLog( "error write file" ); // TODO
      return false;
    }

    left_to_write -= current_write;
    write += current_write;
  } while (write != data.size());

  assert(write == data.size());
  if (write != data.size()) {
    return false;
  }
  fflush(file_);
  return true;
}
