cmake_minimum_required (VERSION 3.15)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(FATAL_ERROR "In-source build is not allowed")
endif()

project(jinject
  LANGUAGES CXX
  HOMEPAGE_URL https://github.com/jeffersonfr/jinject
  DESCRIPTION "A dependency injection library"
  VERSION 1.0.0
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

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

add_subdirectory(tests)

include(Doxygen)

Doxygen(include docs)

install(DIRECTORY include/${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

# generate pc file
configure_file(jinject.pc.in jinject.pc @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/jinject.pc DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
