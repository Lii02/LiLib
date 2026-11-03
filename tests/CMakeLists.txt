file(GLOB LILIB_TEST_SOURCES CONFIGURE_DEPENDS *.cpp *.h)

add_executable(liLibTests ${LILIB_TEST_SOURCES})
target_link_libraries(liLibTests PRIVATE LiLib::LiLib)

lilib_set_target_defaults(liLibTests)

# The timeout turns a deadlock into a test failure instead of a stuck run
add_test(NAME liLibTests COMMAND liLibTests)
set_tests_properties(liLibTests PROPERTIES TIMEOUT 120)
