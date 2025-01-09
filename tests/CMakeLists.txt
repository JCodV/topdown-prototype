add_executable(main_test main_test.cpp)
target_include_directories(main_test PRIVATE ../include)
target_include_libraries(
	main_test
	GTest::gtest_main
	raylib
)

include(GoogleTest)
gtest_discover_tests(main_test)
