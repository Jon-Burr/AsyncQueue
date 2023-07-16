namespace AsyncQueue {

    template <std::derived_from<IMessageWriter> T>
        requires std::move_constructible<T>
    MessageManager::MessageManager(std::stop_source ss, T &&writer, MessageLevel outputLvl)
            : MessageManager(ss, std::make_unique<T>(std::move(writer)), outputLvl) {}

    template <std::derived_from<IMessageWriter> T>
        requires std::move_constructible<T>
    MessageManager::MessageManager(T &&writer, MessageLevel outputLvl)
            : MessageManager(
                      std::stop_source(), std::make_unique<T>(std::move(writer)), outputLvl) {}
} // namespace AsyncQueue