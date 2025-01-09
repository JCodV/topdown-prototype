cmake_minimum_required(VERSION 3.22)

project(boss-rush)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
	raylib
	GIT_REPOSITORY "https://github.com/raysan5/raylib.git"
	GIT_TAG "master"
	GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(raylib)

# FetchContent_Declare(
# 	googletest
# 	URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
# )

# set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
# FetchContent_MakeAvailable(googletest)

add_subdirectory(src)
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_link_libraries(${PROJECT_NAME} raylib)

file(COPY assets DESTINATION ${CMAKE_BINARY_DIR})

# Testing
# enable_testing()
# add_subdirectory(tests)
