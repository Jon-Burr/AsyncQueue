add_library(AsyncQueue)
target_sources(AsyncQueue PRIVATE
    Message.cxx
    MessageFormatter.cxx
    MessageManager.cxx
    MessageQueueBuffer.cxx
    MessageQueueStream.cxx
    MessageSource.cxx
    MessageWriter.cxx
)
target_include_directories(AsyncQueue PUBLIC ../include)
target_compile_features(AsyncQueue PUBLIC cxx_std_20)
target_link_libraries(AsyncQueue PUBLIC Threads::Threads)

if(AsyncQueue_MULTITHREAD)
    target_compile_definitions(AsyncQueue PUBLIC AsyncQueue_MULTITHREAD)
endif()