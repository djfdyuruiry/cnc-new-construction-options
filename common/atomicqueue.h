#pragma once

#include <memory>
#include <mutex>
#include <queue>

/**
 * Thread safe queue, useful for message passing.
 *
 * Used with @class{LuaEvent} classes for async Lua
 * calls back to game engine.
 */
template <class T>
class AtomicQueue
{
public:
    AtomicQueue(): Queue() {}

    /**
     * Use the underlying std::queue with exclusive access.
     */
    void Access(std::function<void(std::queue<std::unique_ptr<T>>&)> action)
    {
        std::lock_guard lock(Mutex);

        action(Queue);
    }

    /**
     * Build item of type U and enqueue.
     */
    template<typename U, typename... Args>
    void Push(Args&&... args) {
        std::lock_guard lock(Mutex);

        Queue.push(
            std::make_unique<U>(std::forward<Args>(args)...)
        );
    }
private:
    std::queue<std::unique_ptr<T>> Queue;
    std::mutex Mutex;
};
