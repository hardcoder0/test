cmake_minimum_required(VERSION 3.0.0)
project(pekoscript VERSION 0.1.0)

include(CTest)
enable_testing()

SET(CMAKE_CXX_COMPILER "/usr/bin/clang++")

add_executable(pekoscript "src/main.cxx")
set(CMAKE_CXX_FLAGS "-I/home/preston/dev/peko-objects_done/src/include -I/home/preston/dev/peko-objects_done/external -I/usr/lib/llvm-12/include -std=c++14 -D_GNU_SOURCE -D__STDC_CONSTANT_MACROS -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -L/usr/lib/llvm-12/lib -lLLVM-12")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})