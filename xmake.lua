set_languages("c++23")
add_rules("mode.debug", "mode.release")

function setup_compiler_flags()
  add_cxxflags(
    "-Wall",
    "-Wextra", -- reasonable and standard
    "-Wshadow", -- warn the user if a variable declaration shadows one from a parent context
    "-Wnon-virtual-dtor", -- warn the user if a class with virtual functions has a non-virtual destructor. This helps
    "-Wcast-align", -- warn for potential performance problem casts
    "-Wunused", -- warn on anything being unused
    "-Woverloaded-virtual", -- warn if you overload (not override) a virtual function
    "-Wpedantic", -- warn if non-standard C++ is used
    "-Wconversion", -- warn on type conversions that may lose data
    "-Wsign-conversion", -- warn on sign conversions
    "-Wnull-dereference", -- warn if a null dereference is detected
    "-Wdouble-promotion", -- warn if float is implicit promoted to double
    "-Wformat=2", -- warn on security issues around functions that format output (ie printf)
    "-Wno-strict-prototypes",
    "-Wno-missing-designated-field-initializers"
  )
  if is_plat("windows") then
    add_defines("_CRT_SECURE_NO_WARNINGS", "WIN32_LEAN_AND_MEAN", "NOMINMAX")
  end
  if is_mode("debug") then
    add_defines("DEBUG", "_DEBUG")
  else
    add_defines("NDEBUG", "RELEASE")
  end
end

target("cc")
  set_kind("static")
  add_files("cc/**.cpp", "cc/**.c")
  add_includedirs("cc/inc", {public = true})
  add_includedirs("cc/src", {private = true})
  if is_plat("windows") then
    add_links("Shell32")
  end
  setup_compiler_flags()
  add_tests("cc-tests", {
    kind = "binary",
    files = "cc-tests/**.cpp",
    deps = "cc"
  })
  add_headerfiles("cc/inc/cc/*", {prefixdir = "cc"})
target_end()
