#pragma once

#include <memory>
#include <mutex>
#include <queue>

/**
 * Thread safe queue, useful for message passing.
 *1
 * Used with @class{LuaEvent} classes for async Lua
 * calls back to game engine.
 */
template <class T>
class AtomicQueue {
public:
    AtomicQueue(): Queue(std::make_unique<std::queue<T>>()), Mutex() {}

    /**
     * Use the underlying std::queue with exclusive access.
     */
    void Access(std::function<void(std::unique_ptr<std::queue<T>>&)> action)
    {
        std::lock_guard<std::mutex> lock(Mutex);

        action(Queue);
    }

    /**
     * Build item of type U and enqueue.
     */
    template<typename U, typename... Args>
    void Push(Args&&... args) {
        std::lock_guard<std::mutex> lock(Mutex);

        Queue->push(
            std::move(
                std::make_unique<U>(std::forward<Args>(args)...)
            )
        );
    }
private:
    std::unique_ptr<std::queue<T>> Queue;
    std::mutex Mutex;
};
