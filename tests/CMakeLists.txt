cmake_minimum_required (VERSION 3.0)

set(gtest_force_shared_crt ON)
set(INSTALL_GTEST OFF)
set(INSTALL_GMOCK OFF)
include(FetchContent)
FetchContent_Declare(googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.12.1
)
FetchContent_MakeAvailable(googletest)

include(Coverage)

macro(module_test)
  set(id ${ARGV0}_test)

  add_executable(${id} ${id}.cpp)
  add_test(${id} ${id} COMMAND $<TARGET_FILE:${id}>)
  target_link_libraries(${id}
    PRIVATE
      jinject
      gtest_main
  )

  # EnableCoverage(${id})
  # AddCoverage(${id})

  unset(id)
endmacro()

module_test(unit)
