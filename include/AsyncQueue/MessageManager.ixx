namespace AsyncQueue {

#ifdef AsyncQueue_MULTITHREAD
    template <std::derived_from<IMessageWriter> T>
        requires std::move_constructible<T>
    MessageManager::MessageManager(std::stop_source ss, T &&writer, MessageLevel outputLvl)
            : MessageManager(ss, std::make_unique<T>(std::move(writer)), outputLvl) {}

#endif
    template <std::derived_from<IMessageWriter> T>
        requires std::move_constructible<T>
    MessageManager::MessageManager(T &&writer, MessageLevel outputLvl)
            : MessageManager(std::make_unique<T>(std::move(writer)), outputLvl) {}
} // namespace AsyncQueue