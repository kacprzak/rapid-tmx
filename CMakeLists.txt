cmake_minimum_required(VERSION 3.12)
project(rapid-tmx)

find_package(Boost REQUIRED)
if(UNIX)
  find_package(Boost COMPONENTS unit_test_framework)
endif()
find_package(ZLIB REQUIRED)

add_library(rapid-tmx src/TmxMap.cpp external/base64/base64.cpp)
add_library(rapid-tmx::rapid-tmx ALIAS rapid-tmx)
target_include_directories(rapid-tmx PUBLIC include)
target_include_directories(rapid-tmx PRIVATE external)
target_link_libraries(rapid-tmx PRIVATE Boost::boost ZLIB::ZLIB)

if(Boost_UNIT_TEST_FRAMEWORK_FOUND)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

install(TARGETS rapid-tmx
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib
  )
install(DIRECTORY include/ DESTINATION include)

set(CPACK_PACKAGE_CONTACT martin.kacprzak@gmail.com)
set(CPACK_DEBIAN_PACKAGE_DEPENDS "zlib1g (>= 1:1.2.11.dfsg-1ubuntu2)")

include(CPack)
