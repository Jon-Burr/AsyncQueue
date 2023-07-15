cmake_minimum_required(VERSION 3.12)

project(AsyncQueue VERSION 0.1.0 LANGUAGES CXX)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

# add_subdirectory(src)