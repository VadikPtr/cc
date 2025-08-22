#include "cc/test.hpp"
#include "cc/common.hpp"
#include "cc/fs.hpp"

mTestCase(path_parent) {
  mRequireEqStr(Path("/hello/world").parent(), "/hello");
  mRequireEqStr(Path("C:/hello/world").parent(), "C:/hello");
  mRequireEqStr(Path("C:/world").parent(), "C:");
  mRequireEqStr(Path("/hello").parent(), "");

  mRequireEqStr(Path("//hello\\world//").normalized(), "/hello/world");

  mRequire(!Path("").has_parent());
  mRequire(!Path("/").has_parent());
  mRequire(!Path("/test").has_parent());
  mRequire(Path("/test/dir").has_parent());
  mRequire(Path("C:/test/dir").has_parent());

  mRequireEqStr(Path("a") / Path("b") / Path("c"), "a/b/c");
  mRequireEqStr(Path::join("a", "b", "c"), "a/b/c");
  mRequireEqStr(Path::join("a", "", "c"), "a/c");
  mRequireEqStr(Path::join("a", "", ""), "a");
  mRequireEqStr(Path::join("", "", ""), "");
  mRequireEqStr(Path::join("", "a", "b"), "a/b");
  mRequireEqStr(Path::join("", "a", ""), "a");
  mRequireEqStr(Path::join("", "", "a"), "a");

  mRequireEqStr(Path("test").name(), "test");
  mRequireEqStr(Path("test.txt").name(), "test.txt");
  mRequireEqStr(Path("a/test.txt").name(), "test.txt");
  mRequireEqStr(Path("a///test.txt").name(), "test.txt");
  mRequireEqStr(Path("a///test.txt/").name(), "");
  mRequireEqStr(Path("a///test.txt").name_without_ext(), "test");
  mRequireEqStr(Path("a///test").name_without_ext(), "test");
  mRequireEqStr(Path("a///test.txt.ext").ext(), ".txt.ext");
  mRequireEqStr(Path("test.txt.ext").ext(), ".txt.ext");
  mRequireEqStr(Path("test").ext(), "test");
  mRequire(Path("test") == StrView("test"));
  mRequireEqStr(Path("test.test").with_ext(".txt"), "test.txt");
  mRequireEqStr(Path("test.txt.ext").with_ext(".tar.gz"), "test.tar.gz");
  mRequireEqStr(Path("test").with_ext(".txt"), "test.txt");
  mRequireEqStr(Path("").with_ext(".txt"), ".txt");
  mRequireEqStr(Path("/a.txt/b.txt").with_ext(".dds"), "/a.txt/b.dds");
}

mTestCase(path_components) {
  mRequire(Path("a/b/c").components_count() == 3);
  mRequire(Path("a//b///c").components_count() == 3);
  mRequire(Path("/a/b/").components_count() == 2);
  mRequire(Path("/").components_count() == 0);
  mRequire(Path("").components_count() == 0);
  mRequire(Path("single").components_count() == 1);
  mRequire(Path("///").components_count() == 0);
  mRequire(Path("dir/file.txt").components_count() == 2);

  {
    StrView data[3];
    Path    path("a/b/c");
    mRequire(path.get_components(data) == 3);
    mRequireEqStr(data[0], "a");
    mRequireEqStr(data[1], "b");
    mRequireEqStr(data[2], "c");
  }

  {
    StrView data[3];
    Path    path("a//b///c");
    mRequire(path.get_components(data) == 3);
    mRequireEqStr(data[0], "a");
    mRequireEqStr(data[1], "b");
    mRequireEqStr(data[2], "c");
  }

  {
    StrView data[2];
    Path    path("/a/b/");
    mRequire(path.get_components(data) == 2);
    mRequireEqStr(data[0], "a");
    mRequireEqStr(data[1], "b");
  }

  {
    Path path("/");
    mRequire(path.get_components(ArrView<StrView>{}) == 0);
  }

  {
    Path path("");
    mRequire(path.get_components(ArrView<StrView>{}) == 0);
  }

  {
    Path path("///");
    mRequire(path.get_components(ArrView<StrView>{}) == 0);
  }

  {
    StrView data[1];
    Path    path("single");
    mRequire(path.get_components(data) == 1);
    mRequireEqStr(data[0], "single");
  }
}

mTestCase(path_relative) {
  mRequireEqStr(Path("a/b/c").relative_to(Path("/a/b")), "c");
  mRequireEqStr(Path("a").relative_to(Path("a")), "");
  mRequireEqStr(Path("a/b/c").relative_to(Path("c/d/e")), "../../../a/b/c");
}
