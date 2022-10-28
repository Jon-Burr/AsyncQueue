#ifndef ASYNCQUEUE_SYNCSTREAM_H
#define ASYNCQUEUE_SYNCSTREAM_H

#include <memory>
#include <mutex>
#include <ostream>
#include <type_traits>

/**
 * NB These classes are mainly to make up for the lack of <syncstream> in C++17...
 *
 * My implementation here is bare bones and nowhere near as safe as the library implementation
 */

namespace AsyncQueue {

    class SyncStream {
    public:
        static SyncStream cout();
        static SyncStream cerr();

        static std::shared_ptr<std::mutex> getMutex(const std::ostream &os);
        SyncStream(std::ostream &os);
        SyncStream(const SyncStream &os);

        void flush();

        template <typename T> SyncStream &operator<<(T &&value) {
            if (!m_lock.owns_lock())
                m_lock.lock();
            m_os << std::forward<T>(value);
            return *this;
        }

        using stream_mod_t = std::ostream &(*)(std::ostream &);
        SyncStream &operator<<(stream_mod_t mod) {
            if (!m_lock.owns_lock())
                m_lock.lock();
            m_os << mod;
            if (mod == &std::endl<std::ostream::char_type, std::ostream::traits_type>)
                m_lock.unlock();
            return *this;
        }

    private:
        std::ostream &m_os;
        std::shared_ptr<std::mutex> m_mutex;
        std::unique_lock<std::mutex> m_lock;
    };

} // namespace AsyncQueue

#endif //> !ASYNCQUEUE_SYNCSTREAM_H