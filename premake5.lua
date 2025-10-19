---@diagnostic disable: undefined-field, undefined-global, lowercase-global

warnings_flags = {
  "-Werror",
  "-Wall",
  "-Wextra",
  "-Wshadow",
  "-Wnon-virtual-dtor",
  "-Wcast-align",
  "-Wunused",
  "-Woverloaded-virtual",
  "-Wpedantic",
  "-Wconversion",
  "-Wsign-conversion",
  "-Wnull-dereference",
  "-Wdouble-promotion",
  "-Wformat=2",
  "-Wno-strict-prototypes",
  "-Wno-missing-designated-field-initializers",
}

root_dir = os.getcwd()

workspace "cc"
  location "build"
  buildoptions { "-fdiagnostics-color=always", "-arch arm64" }
  configurations { "Debug", "Release" }
  cppdialect "C++20"
  cdialect "C11"
  targetdir "bin/%{cfg.buildcfg}"
  linkoptions { "-rpath @executable_path" }

  filter "configurations:Debug"
    defines { "DEBUG" }
    symbols "On"
    optimize "Off"

  filter "configurations:Release"
    defines { "NDEBUG" }
    optimize "Full"
    linkoptions { "-flto" }

include "cc"

project "cc-tests"
  kind "ConsoleApp"
  buildoptions { warnings_flags }
  language "C++"
  link_cc()
  files { "cc-tests/**.hpp", "cc-tests/**.cpp" }
  includedirs { "cc-tests/src" }
