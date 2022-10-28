#include "AsyncQueue/SyncStream.h"

#include <iostream>
#include <map>

namespace AsyncQueue {

    SyncStream SyncStream::cout() { return SyncStream(std::cout); }
    SyncStream SyncStream::cerr() { return SyncStream(std::cerr); }

    SyncStream::SyncStream(const SyncStream &other)
            : m_os(other.m_os), m_mutex(other.m_mutex), m_lock(*m_mutex, std::defer_lock) {}

    std::shared_ptr<std::mutex> SyncStream::getMutex(const std::ostream &os) {
        static std::mutex mapMutex;
        std::lock_guard<std::mutex> lock(mapMutex);
        static std::map<const std::ostream *, std::weak_ptr<std::mutex>> mutexMap;
        auto itr = mutexMap.find(&os);
        if (itr == mutexMap.end() || itr->second.expired()) {
            auto ptr = std::make_shared<std::mutex>();
            mutexMap[&os] = ptr;
            return ptr;
        } else
            return itr->second.lock();
    }

    void SyncStream::flush() {
        if (!m_lock.owns_lock())
            m_lock.lock();
        m_os.flush();
        m_lock.unlock();
    }

    SyncStream::SyncStream(std::ostream &os)
            : m_os(os), m_mutex(getMutex(os)), m_lock(*m_mutex, std::defer_lock) {}
} // namespace AsyncQueue