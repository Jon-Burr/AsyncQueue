add_library(AsyncQueue)
target_sources(AsyncQueue PRIVATE
    FileMessageWriter.cxx
    Message.cxx
    MessageFormatter.cxx
    MessageManager.cxx
    MessageSource.cxx
    MessageWriter.cxx
    ThreadManager.cxx
)
target_include_directories(AsyncQueue PUBLIC ../include)
target_compile_features(AsyncQueue PUBLIC cxx_std_17)
target_link_libraries(AsyncQueue PUBLIC Threads::Threads)