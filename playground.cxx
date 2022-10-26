#include "AsyncQueue/AsyncQueue.h"
#include "AsyncQueue/ThreadManager.h"

#include <chrono>
#include <iostream>
#include <thread>

void printQueue(AsyncQueue::AsyncQueue<std::string> &queue, bool &stop) {
    while (!stop) {
        auto lock = queue.lock();
        while (std::optional<std::string> value = queue.extract(lock)) {
            // Unlock while we deal with the value
            lock.unlock();
            std::cout << *value << std::endl;
            // Reacquire the lock before trying to extract another value
            lock.lock();
        }
        queue.cv().wait(lock);
    }
}

void printMessage(AsyncQueue::AsyncQueue<std::string> &queue) {
    auto lock = queue.lock();
    while (std::optional<std::string> value = queue.extract(lock)) {
        // Unlock while we deal with the value
        lock.unlock();
        std::cout << *value << std::endl;
        // Reacquire the lock before trying to extract another value
        lock.lock();
    }
    queue.cv().wait(lock);
}

void queueMessage(
        AsyncQueue::AsyncQueue<std::string> &queue, bool &stop, std::string message, float delay) {
    using namespace std::chrono_literals;
    while (!stop) {
        queue.push(message);
        std::this_thread::sleep_for(1s * delay);
    }
}

void enqueue(AsyncQueue::AsyncQueue<std::string> &queue, std::string message, float delay) {
    using namespace std::chrono_literals;
    queue.push(message);
    std::this_thread::sleep_for(1s * delay);
}

int main() {
    using namespace std::chrono_literals;
    bool stop = false;
    AsyncQueue::AsyncQueue<std::string> queue;
    AsyncQueue::ThreadManager mgr;
    // std::future<void> writeFuture = mgr.template loop(queue.cv(), printMessage, std::ref(queue));
    // std::future<void> msgFuture1 = mgr.loop(enqueue, std::ref(queue), "Hello", 0.5);
    // std::future<void> msgFuture2 = mgr.loop(enqueue, std::ref(queue), "Goodbye", 1);
    mgr.loop(&enqueue, queue, "Hello", 0.5f);
    // mgr.loop(&enqueue, queue, "Goodbye", 1);
    std::this_thread::sleep_for(10s);
    mgr.abort();
    // writeFuture.wait();
    // msgFuture1.wait();
    // msgFuture2.wait();
    return 0;
}