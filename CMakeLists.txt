cmake_minimum_required(VERSION 3.12)
project(rapid-tmx)

find_package(Boost REQUIRED)
if(UNIX)
  find_package(Boost COMPONENTS unit_test_framework)
endif()
find_package(ZLIB REQUIRED)

add_library(rapid-tmx src/TmxMap.cpp external/base64/base64.cpp)
target_include_directories(rapid-tmx PUBLIC include)
target_include_directories(rapid-tmx PRIVATE external)
target_link_libraries(rapid-tmx PRIVATE Boost::boost ZLIB::ZLIB)

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()
