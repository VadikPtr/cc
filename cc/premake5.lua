---@diagnostic disable: undefined-field, undefined-global, lowercase-global

local dir = os.getcwd()

function link_cc()
  includedirs { path.join(dir, "inc") }
  links { "cc" }
end

project "cc"
  kind "StaticLib"
  language "C++"
  buildoptions { warnings_flags, "-Wno-error=deprecated-declarations", }
  files { "**.hpp", "**.cpp", "**.h", "**.c" }
  includedirs { "inc", "src" }
