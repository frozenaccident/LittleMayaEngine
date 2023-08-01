/*
* An event system where each thread has its own queue,
* includes a std::unordered_map that maps thread IDs to event queues.
* 
* In the pushEvent method, the thread's ID is used to look up the corresponding queue in the eventQueues map.
* If no queue exists for that thread ID, one is automatically created.
* The eventQueues map is shared by all threads, so access to it is protected by a mutex.
* 
* In the dispatch method, all queues are processed, not just the one for the current thread.
* This ensures that all events are processed, even if some threads stop pushing events before others.
*/

#include "EventSystem.h"

void EventSystem::addListener(std::type_index type, std::weak_ptr<EventListener> listener) {
    std::lock_guard<std::mutex> lock(mtx);
    listeners.insert(std::make_pair(type, listener));
}

void EventSystem::removeListener(std::weak_ptr<EventListener> listener) {
    std::lock_guard<std::mutex> lock(mtx);
    auto l = listener.lock();
    for (auto it = listeners.begin(); it != listeners.end();) {
        if (auto listener = it->second.lock()) {
            if (listener == l) {
                it = listeners.erase(it);
                continue;
            }
        }
        else {
            // Clean up after listeners that have been deleted
            it = listeners.erase(it);
            continue;
        }
        ++it;
    }
}

void EventSystem::pushEvent(std::shared_ptr<Event> event) {
    std::lock_guard<std::mutex> lock(mtx);
    auto& queue = eventQueues[std::this_thread::get_id()];
    queue.push(event);
}

void EventSystem::dispatch() {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& pair : eventQueues) {
        auto& queue = pair.second;
        while (!queue.empty()) {
            auto event = queue.top();
            auto range = listeners.equal_range(std::type_index(typeid(*event)));
            for (auto it = range.first; it != range.second; ++it) {
                if (auto listener = it->second.lock()) {
                    if (listener->canHandle(event)) {
                        bool handled = listener->onEvent(event);
                        if (handled) {
                            break;
                        }
                    }
                }
            }
            queue.pop();
        }
    }
}
