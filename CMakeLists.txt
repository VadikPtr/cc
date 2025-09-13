cmake_minimum_required(VERSION 3.21)
project(cc-project)

if(NOT VY_DEPS)
  if(PROJECT_IS_TOP_LEVEL)
    set(VY_DEPS ${CMAKE_CURRENT_LIST_DIR}/deps CACHE INTERNAL "vy_deps")
  else()
    set(VY_DEPS ${CMAKE_CURRENT_LIST_DIR}/.. CACHE INTERNAL "vy_deps")
  endif()

  set(VY_ROOT ${VY_DEPS}/.. CACHE INTERNAL "vy_root")
  set(VY_BIN ${VY_ROOT}/bin CACHE INTERNAL "vy_bin")
  set(VY_CC_ROOT ${CMAKE_CURRENT_LIST_DIR} CACHE INTERNAL "vy_cc_root")
endif()

include(cmake/vy.cmake)

add_subdirectory(cc)

if(PROJECT_IS_TOP_LEVEL)
  add_subdirectory(cc-tests)
endif()
