/// @file Fwd.hxx
/// @brief Some useful forward declarations to simplify the code

#ifndef ASYNCQUEUE_FWD_HXX
#define ASYNCQUEUE_FWD_HXX

namespace AsyncQueue {
    class Message;
    template <typename T> class AsyncQueue;
    using MessageQueue = AsyncQueue<Message>;
    template <typename T> class IConsumer;
    using IMessageWriter = IConsumer<Message>;
} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_FWD_HXX