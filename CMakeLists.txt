cmake_minimum_required(VERSION 3.12)
project(rapid-tmx
  VERSION 0.1.1
  HOMEPAGE_URL https://github.com/kacprzak/rapid-tmx
  LANGUAGES CXX
  )

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
  external
  )
set_target_properties(rapid-tmx PROPERTIES PUBLIC_HEADER "include/TmxMap.h")
target_link_libraries(rapid-tmx PRIVATE Boost::boost ZLIB::ZLIB)

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

install(TARGETS rapid-tmx EXPORT ${PROJECT_NAME}-targets
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  RUNTIME DESTINATION bin
  PUBLIC_HEADER DESTINATION include
  )
install(EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-targets.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION lib/cmake/${PROJECT_NAME}
  )

include(CMakePackageConfigHelpers)
write_basic_package_version_file(${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
  )

install(FILES
  ${PROJECT_NAME}-config.cmake
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
  DESTINATION lib/cmake/${PROJECT_NAME}
  )

set(CPACK_PACKAGE_CONTACT martin.kacprzak@gmail.com)
# autogenerate dependency information
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
