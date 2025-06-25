cmake_minimum_required (VERSION 3.15)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project(jinject
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jinject
  DESCRIPTION "A dependency injection helper"
  VERSION 1.0.0
)

option(JINJECT_TESTS "Enable unit tests" OFF)
option(JINJECT_CHECKER "Enable static code analysing" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (JINJECT_CHECKER)
   set(CMAKE_CXX_CLANG_TIDY clang-tidy -checks=-*,android-*,bugprone-*,clang-analyzer-*,concurrency-*,hicpp-*,misc-*,modernize-*,portability-*)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

find_package(Threads REQUIRED)

include(FetchContent)

# jmixin
FetchContent_Declare(
  jmixin
  GIT_REPOSITORY https://github.com/jeffersonfr/jmixin.git
)
FetchContent_MakeAvailable(jmixin)

add_library(${PROJECT_NAME} INTERFACE
  )

target_link_libraries(${PROJECT_NAME}
    INTERFACE
      jmixin
)

target_include_directories(${PROJECT_NAME}
  INTERFACE 
    ${PROJECT_SOURCE_DIR}/include
    jmixin
  )


if (JINJECT_TESTS)
  enable_testing()
  
  add_subdirectory(tests)
endif()

include(Doxygen)

Doxygen(include docs)

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# generate pc file
configure_file(jinject.pc.in jinject.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/jinject.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
