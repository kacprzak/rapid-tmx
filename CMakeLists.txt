cmake_minimum_required(VERSION 3.12)
project(rapid-tmx)

find_package(Boost REQUIRED)
if(UNIX)
  find_package(Boost COMPONENTS unit_test_framework)
endif()
find_package(ZLIB REQUIRED)

add_library(rapid-tmx TmxMap.cpp base64/base64.cpp)
target_include_directories(rapid-tmx PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(rapid-tmx PRIVATE Boost::boost ZLIB::ZLIB)

# add_test_exec
macro(add_test_exec test_name files libs)
  add_executable(${test_name} ${files})
  target_link_libraries(${test_name} ${libs} Boost::unit_test_framework)
  add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}>)
endmacro(add_test_exec)


if( Boost_UNIT_TEST_FRAMEWORK_FOUND )
  include(CTest)
  enable_testing()
  add_test_exec(map_test map_test.cpp rapid-tmx)
endif()
