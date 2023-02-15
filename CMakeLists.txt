cmake_minimum_required(VERSION 3.12)

project(AsyncQueue VERSION 0.0.0 LANGUAGES CXX)

add_library(AsyncQueue)
target_sources(AsyncQueue PRIVATE
    src/FileMessageWriter.cxx
    src/Message.cxx
    src/MessageFormatter.cxx
    src/MessageManager.cxx
    src/MessageSource.cxx
    src/MessageWriter.cxx
    src/TeeWriter.cxx
    src/ThreadManager.cxx)
target_include_directories(AsyncQueue PUBLIC include)
target_link_libraries(AsyncQueue PUBLIC pthread)
target_compile_features(AsyncQueue PUBLIC cxx_std_17)
