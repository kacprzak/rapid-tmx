cmake_minimum_required(VERSION 3.12)
project(rapid-tmx VERSION 0.1.1)

set(CMAKE_DEBUG_POSTFIX "-d")
option(BUILD_SHARED_LIBS "Build shared libraries." OFF)

find_package(Boost REQUIRED)
if(UNIX)
  find_package(Boost COMPONENTS unit_test_framework)
endif()
find_package(ZLIB REQUIRED)

add_library(rapid-tmx src/TmxMap.cpp external/base64/base64.cpp)
add_library(rapid-tmx::rapid-tmx ALIAS rapid-tmx)
target_include_directories(rapid-tmx
  PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
  PRIVATE
  external)
set_target_properties(rapid-tmx PROPERTIES PUBLIC_HEADER "include/TmxMap.h")
target_link_libraries(rapid-tmx PRIVATE Boost::boost ZLIB::ZLIB)

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

install(TARGETS rapid-tmx EXPORT rapid-tmx-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
  )
install(EXPORT rapid-tmx-targets
  FILE rapid-tmx-targets.cmake
  NAMESPACE rapid-tmx::
  DESTINATION lib/cmake/rapid-tmx
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(rapid-tmx-config-version.cmake
  VERSION ${rapid-tmx_VERSION}
  COMPATIBILITY SameMajorVersion
  )

install(FILES
  rapid-tmx-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/rapid-tmx-config-version.cmake
  DESTINATION lib/cmake/rapid-tmx)

set(CPACK_PACKAGE_CONTACT martin.kacprzak@gmail.com)
# autogenerate dependency information
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
