# VY_ROOT - root directory, where main cmake file lives
# VY_DEPS - root directory / deps
# VY_BIN  - root directory / bin

message("VY_DEPS: ${VY_DEPS}")
message("VY_ROOT: ${VY_ROOT}")
message("VY_BIN:  ${VY_BIN}")

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  set(vy_is_macos TRUE CACHE INTERNAL "vy_is_macos")
  set(vy_is_windows FALSE CACHE INTERNAL "vy_is_windows")
  set(vy_is_linux FALSE CACHE INTERNAL "vy_is_linux")
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set(vy_is_macos FALSE CACHE INTERNAL "vy_is_macos")
  set(vy_is_windows FALSE CACHE INTERNAL "vy_is_windows")
  set(vy_is_linux TRUE CACHE INTERNAL "vy_is_linux")
else()
  set(vy_is_macos FALSE CACHE INTERNAL "vy_is_macos")
  set(vy_is_windows TRUE CACHE INTERNAL "vy_is_windows")
  set(vy_is_linux FALSE CACHE INTERNAL "vy_is_linux")
endif()

function(vy_dep_subdirectory)
  set(options)
  set(oneValueArgs NAME TARGET)
  set(multiValueArgs FEATURES)
  cmake_parse_arguments(VY_ADD_LIBRARY "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT DEFINED VY_ADD_LIBRARY_NAME)
    message(FATAL_ERROR "vy_add_library() called with no NAME parameter")
  endif()

  if(NOT DEFINED VY_ADD_LIBRARY_TARGET)
    message(DEBUG "vy_add_library() called with no TARGET parameter, assuming name as target")
    set(VY_ADD_LIBRARY_TARGET ${VY_ADD_LIBRARY_NAME})
  endif()

  if(NOT TARGET ${VY_ADD_LIBRARY_TARGET})
    message("vy_add_library: ${VY_DEPS}/${VY_ADD_LIBRARY_NAME} (features: '${VY_ADD_LIBRARY_FEATURES}')")
    if(NOT DEFINED vy_target_features_${VY_ADD_LIBRARY_NAME})
      set(
        vy_target_features_${VY_ADD_LIBRARY_NAME} ${VY_ADD_LIBRARY_FEATURES}
        CACHE INTERNAL "vy_target_features_${VY_ADD_LIBRARY_NAME}"
      )
    endif()
    add_subdirectory(${VY_DEPS}/${VY_ADD_LIBRARY_NAME} ${VY_ADD_LIBRARY_NAME})
  endif()
endfunction()

function(vy_set_target_output_variant target suffix variant)
  if(variant STREQUAL "")
    set_target_properties(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${VY_BIN}/${suffix}
      PDB_OUTPUT_DIRECTORY ${VY_BIN}/${suffix}
      LIBRARY_OUTPUT_DIRECTORY ${VY_BIN}/${suffix}
    )
  else()
    set_target_properties(${target} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY_${variant} ${VY_BIN}/${suffix}
      PDB_OUTPUT_DIRECTORY_${variant} ${VY_BIN}/${suffix}
      LIBRARY_OUTPUT_DIRECTORY_${variant} ${VY_BIN}/${suffix}
    )
  endif()
endfunction()

macro(vy_set_target_output target)
  vy_set_target_output_variant(${target} "debug" "")
  vy_set_target_output_variant(${target} "debug" "DEBUG")
  vy_set_target_output_variant(${target} "release" "RELEASE")
  vy_set_target_output_variant(${target} "release" "RELWITHDEBINFO")
  vy_set_target_output_variant(${target} "release" "MINSIZEREL")
endmacro()

function(vy_configure_compiler target)
  set_target_properties(${target} PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    C_STANDARD 11
    INTERPROCEDURAL_OPTIMIZATION $<IF:$<CONFIG:Release>,ON,OFF>
  )

  set(sanitize_opts "address,undefined,bounds,alignment,null")

  if(vy_is_windows)
    set(sanitize_opts "address,undefined")
    target_compile_definitions(${target} PRIVATE
      _CRT_SECURE_NO_WARNINGS NOMINMAX WIN32_LEAN_AND_MEAN _MBCS
    )
    set_target_properties(${target} PROPERTIES
      MSVC_RUNTIME_LIBRARY $<IF:$<CONFIG:RelWithDebInfo>,MultiThreaded,MultiThreaded$<$<CONFIG:Debug>:Debug>DLL>
    )
  endif()

  if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE
      -Wall
      -Wextra # reasonable and standard
      -Wshadow # warn the user if a variable declaration shadows one from a parent context
      -Wnon-virtual-dtor # warn the user if a class with virtual functions has a non-virtual destructor. This helps
      -Wcast-align # warn for potential performance problem casts
      -Wunused # warn on anything being unused
      -Woverloaded-virtual # warn if you overload (not override) a virtual function
      -Wpedantic # warn if non-standard C++ is used
      -Wconversion # warn on type conversions that may lose data
      -Wsign-conversion # warn on sign conversions
      -Wnull-dereference # warn if a null dereference is detected
      -Wdouble-promotion # warn if float is implicit promoted to double
      -Wformat=2 # warn on security issues around functions that format output (ie printf)
      -Wno-strict-prototypes
      -Wno-missing-designated-field-initializers
      $<$<CONFIG:RelWithDebInfo>:-fsanitize=${sanitize_opts} -fstack-protector-strong>
      $<$<NOT:$<CONFIG:Release>>:-g>
    )
    target_link_options(${target} PRIVATE
      $<$<CONFIG:RelWithDebInfo>:-fsanitize=${sanitize_opts}>
    )
    #  target_link_options(${target} PRIVATE /NODEFAULTLIB:msvcrtd /NODEFAULTLIB:msvcrt)
  endif()

  target_compile_definitions(${target} PRIVATE
    $<$<CONFIG:Debug>:_DEBUG DEBUG>
    $<$<NOT:$<CONFIG:Debug>>:NDEBUG>
  )
  vy_set_target_output(${target})
endfunction()
