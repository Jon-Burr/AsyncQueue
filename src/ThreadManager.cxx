#include "AsyncQueue/ThreadManager.hxx"
#include "AsyncQueue/utils.hxx"

namespace AsyncQueue {
    ThreadManager::ThreadManager(MessageSource &&msg)
            : MessageComponent(std::move(msg)), m_abortFuture(m_abortPromise.get_future()) {}

    bool ThreadManager::isAborted() const { return isAborted(std::shared_lock(m_abortMutex)); }

    void ThreadManager::abort() {
        // Acquire a lock on the abort mutex to make sure that nothing can abort this halfway
        // through the call
        auto lock = std::unique_lock(m_abortMutex);
        // Make sure we haven't already aborted
        if (isAborted(lock))
            return;
        m_abortPromise.set_value();
        // Also notify any CVs
        {
            auto cvLock_ = std::unique_lock(m_cvMutex);
            for (auto &cvPair : m_cvRefCounter)
                cvPair.first->notify_all();
        }
        {
            auto cvLock_ = std::unique_lock(m_cvAnyMutex);
            for (auto &cvPair : m_cvAnyRefCounter)
                cvPair.first->notify_all();
        }
    }

    template <typename CV> ThreadManager::CVReference<CV>::~CVReference() {
        if (m_cv)
            m_mgr->decreaseRefCounter(m_cv);
    }

    template <typename CV>
    ThreadManager::CVReference<CV>::CVReference(CVReference &&other)
            : m_mgr(other.m_mgr), m_cv(other.m_cv) {
        other.m_cv = nullptr;
    }

    template <typename CV>
    ThreadManager::CVReference<CV>::CVReference(ThreadManager *mgr, CV *cv) : m_mgr(mgr), m_cv(cv) {
        m_mgr->increaseRefCounter(m_cv);
    }

    ThreadManager::CVReference<std::condition_variable> ThreadManager::registerConditionVariable(
            std::condition_variable &cv) {
        return CVReference(this, &cv);
    }

    ThreadManager::CVReference<std::condition_variable_any> ThreadManager::
            registerConditionVariable(std::condition_variable_any &cv) {
        return CVReference(this, &cv);
    }

    bool ThreadManager::isAborted(const std::unique_lock<std::shared_mutex> &) const {
        return isFutureReady(m_abortFuture);
    }

    bool ThreadManager::isAborted(const std::shared_lock<std::shared_mutex> &) const {
        return isFutureReady(m_abortFuture);
    }

    void ThreadManager::increaseRefCounter(std::condition_variable *cv) {
        auto lock = std::unique_lock(m_cvMutex);
        ++m_cvRefCounter[cv];
    }

    void ThreadManager::increaseRefCounter(std::condition_variable_any *cv) {
        auto lock = std::unique_lock(m_cvAnyMutex);
        ++m_cvAnyRefCounter[cv];
    }

    void ThreadManager::decreaseRefCounter(std::condition_variable *cv) {
        auto lock = std::unique_lock(m_cvMutex);
        auto itr = m_cvRefCounter.find(cv);
        if (itr == m_cvRefCounter.end())
            throw std::out_of_range("Cannot deregister unregistered CV!");
        if (--(itr->second) == 0)
            m_cvRefCounter.erase(itr);
    }

    void ThreadManager::decreaseRefCounter(std::condition_variable_any *cv) {
        auto lock = std::unique_lock(m_cvAnyMutex);
        auto itr = m_cvAnyRefCounter.find(cv);
        if (itr == m_cvAnyRefCounter.end())
            throw std::out_of_range("Cannot deregister unregistered CV!");
        if (--(itr->second) == 0)
            m_cvAnyRefCounter.erase(itr);
    }

    template class ThreadManager::CVReference<std::condition_variable>;
    template class ThreadManager::CVReference<std::condition_variable_any>;

} // namespace AsyncQueue