cmake_minimum_required(VERSION 3.12)

project(AsyncQueue VERSION 0.0.0 LANGUAGES CXX)

add_library(AsyncQueue)
target_sources(AsyncQueue PRIVATE src/ThreadManager.cxx)
target_include_directories(AsyncQueue PUBLIC include)
target_link_libraries(AsyncQueue PUBLIC pthread)
target_compile_features(AsyncQueue PUBLIC cxx_std_17)

add_executable(playground playground.cxx)
target_link_libraries(playground PRIVATE AsyncQueue)
add_executable(tester tester.cxx)
target_link_libraries(tester PRIVATE pthread)
target_compile_features(tester PRIVATE cxx_std_17)
add_executable(test2 test2.cxx)
target_compile_features(test2 PRIVATE cxx_std_17)