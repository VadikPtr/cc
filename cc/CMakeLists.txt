project(cc)
file(GLOB_RECURSE CC_SOURCES *.cpp *.hpp *.c *.h)
add_library(cc STATIC ${CC_SOURCES})
vy_configure_compiler(cc)
target_include_directories(cc
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
  PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)
