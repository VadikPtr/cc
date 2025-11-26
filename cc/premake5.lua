---@diagnostic disable: undefined-field, undefined-global, lowercase-global

local dir = os.getcwd()

function link_cc()
  includedirs { path.join(dir, "inc") }
  links { "cc" }
  if os.istarget("windows") then
    links { "Shell32" }
  end
end

project "cc"
  kind "StaticLib"
  language "C++"
  files { "**.hpp", "**.cpp", "**.h", "**.c" }
  includedirs { "inc", "src" }
  depo_warnings()
