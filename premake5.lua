---@diagnostic disable: undefined-field, undefined-global, lowercase-global

root_dir = os.getcwd()

depo_workspace("cc")
depo_warnings()

include "cc"

project "cc-tests"
  kind "ConsoleApp"
  language "C++"
  link_cc()
  files { "cc-tests/**.hpp", "cc-tests/**.cpp" }
  includedirs { "cc-tests/src" }
