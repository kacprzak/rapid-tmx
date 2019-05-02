macro(add_test_exec test_name files libs)
  add_executable(${test_name} ${files})
  target_link_libraries(${test_name} ${libs} Boost::unit_test_framework)
  add_test(NAME ${test_name} COMMAND $<TARGET_FILE:${test_name}> WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endmacro(add_test_exec)

add_test_exec(map_test map_test.cpp rapid-tmx)
