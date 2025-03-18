cmake_minimum_required(VERSION 3.16)
project(RegexEngine VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -Werror")

# find_program(CLANG_TIDY NAMES clang-tidy)
# if(CLANG_TIDY)
    # set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY}")
# endif()

set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/third_party)
include(FetchContent)

FetchContent_Declare(
  asio
  GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_TAG asio-1-28-0
)
FetchContent_MakeAvailable(asio)
# asio 没有 CMakeLists.txt 适配 FetchContent_MakeAvailable
# 主要是一个 header-only 库, 并没有 CMake 自带的安装规则
# 所以需要手动 include_directories
include_directories(${asio_SOURCE_DIR}/asio/include)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
  fmt
  GIT_REPOSITORY https://github.com/fmtlib/fmt.git
  GIT_TAG 10.2.1
)
FetchContent_MakeAvailable(fmt)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/include/netw)

add_subdirectory(src)
# enable_testing() must be called before add_subdirectory(tests)
enable_testing()
add_subdirectory(tests)
# add_subdirectory(tools)
