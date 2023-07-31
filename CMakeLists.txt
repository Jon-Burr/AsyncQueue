cmake_minimum_required(VERSION 3.12)

project(AsyncQueue VERSION 0.1.0 LANGUAGES CXX)

# Many things which depend on this library *can* be run in non-multithreaded
# environments, but they still depend on this library. This can introduce problems as
# (in particular) Apple's version of Clang runs a long way behind and does not support
# libjthread or std::stop_tokens. Therefore we also allow the library to provide an
# extremely pared down interface that is single threaded and only really exposes the
# messaging side of the library. This is exposed to CMake and C++ via the variable
# AsyncQueue_MULTITHREAD (as a preprocessor macro in C++) which is 1 when
# multithreaded support is available
include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("stop_token" AsyncQueue_MULTITHREAD -std=c++20)

if(${AsyncQueue_MULTITHREAD})
    message(STATUS "stop_token include found: multithreading is enabled")
else()
    message(STATUS "stop_token include not found: multithreading is not enabled")
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

add_subdirectory(src)