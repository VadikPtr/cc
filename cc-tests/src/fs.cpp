#include "cc/test.hpp"
#include "cc/all.hpp"

namespace {
  struct PrintRelativeVisitor final : IFileVisitor {
    Path& base_dir;

    explicit PrintRelativeVisitor(Path& base_dir) : base_dir(base_dir) {}

    bool visit(const Path& path, FsType type) override {
      if (type == FsType::Directory) {
        mLogInfo("open dir: ", path);
      } else {
        auto relative = path.relative_to(base_dir);
        mLogInfo("  ", relative);
      }
      return true;
    }

    bool visit_dir_end(const Path& path) override {
      mLogInfo("close dir: ", path);
      return true;
    }
  };
}  // namespace

mTestCase(fs_example) {
  mLogInfo("type 'unknown_file' = ", "unknown_file"_p.type());
  mLogInfo("type '/' = ", "/"_p.type());
  mLogInfo("type '/bin' = ", "/"_p.type());
  mLogInfo("type 'C:/' = ", "C:/"_p.type());
  mLogInfo("type 'E:/src' = ", "E:/src"_p.type());
  mLogInfo("type 'cc-tests' = ", "cc-tests"_p.type());
  mLogInfo("type 'cc-tests.exe' = ", "cc-tests.exe"_p.type());
  mLogInfo("size 'cc-tests' = ", HumanMemorySize("cc-tests"_p.file_size()));
  mLogInfo("size 'cc-tests.exe' = ", HumanMemorySize("cc-tests.exe"_p.file_size()));
  mLogInfo("size 'bin/debug/cc-tests' = ",
           HumanMemorySize("bin/debug/cc-tests"_p.file_size()));
  mLogInfo("size 'bin/release/cc-tests' = ",
           HumanMemorySize("bin/release/cc-tests"_p.file_size()));

  auto executable_path = Path::to_exe();
  auto current_path    = Path::to_cwd();
  mLogInfo("executable_path = ", executable_path);
  mLogInfo("current_path = ", current_path);

  auto bin_dir = executable_path.parent().parent().absolute();

  auto example_dir = bin_dir / "example_dir";
  mRequire(example_dir.type() == FsType::NoExists);
  mLogInfo("create dir: ", example_dir);
  example_dir.create_dir();
  mRequire(example_dir.type() == FsType::Directory);
  mFinalAction(example_dir, {
    mLogInfo("remove dir: ", example_dir);
    example_dir.remove_dir(FsDirMode::Recursive);
  });

  File f(example_dir / "file.txt", "wb");
  f.write_bytes(to_bytes("Test"_sv));
  f.close();

  auto visitor = PrintRelativeVisitor(bin_dir);
  mLogInfo("visiting directory ", bin_dir);
  bin_dir.visit_dir(visitor, FsDirMode::Recursive);
}

mTestCase(fs_write_read_file) {
  auto path = Path::to_exe().parent() / "test-file.txt"_sv;

  {
    File f;
    Str  write_data(5_kb);
    Str  read_data(5_kb);
    for (size_t i = 0; i < write_data.size(); ++i) {
      write_data[i] = char(size_t('A') + i % size_t('z' - 'A'));
    }

    f.open(path, "wb");
    f.write_bytes(to_bytes(write_data));

    f.open(path, "rb");
    f.read_bytes(to_bytes(read_data));
    mRequireEqStr(write_data, read_data);
  }

  path.remove_file();
}
