#pragma once

#include <unordered_map>
#include <typeindex>
#include <queue>
#include <vector>
#include <memory>
#include <mutex>
#include <thread>

class Event {
public:
    Event(int priority = 0) : priority(priority) {}
    virtual ~Event() {}
    int getPriority() const { return priority; }
private:
    int priority;
};

class EventListener {
public:
    virtual ~EventListener() {}
    virtual bool onEvent(const std::shared_ptr<Event>& event) = 0;
    virtual bool canHandle(const std::shared_ptr<Event>& event) = 0;
};

struct CompareEventPriority {
    bool operator()(const std::shared_ptr<Event>& lhs, const std::shared_ptr<Event>& rhs) const {
        return lhs->getPriority() < rhs->getPriority();
    }
};

class EventSystem {
public:
    void addListener(std::type_index type, std::weak_ptr<EventListener> listener);
    void removeListener(std::weak_ptr<EventListener> listener);
    void pushEvent(std::shared_ptr<Event> event);
    void dispatch();
private:
    std::unordered_multimap<std::type_index, std::weak_ptr<EventListener>> listeners;
    std::unordered_map<std::thread::id, std::priority_queue<std::shared_ptr<Event>, std::vector<std::shared_ptr<Event>>, CompareEventPriority>> eventQueues;
    std::mutex mtx;
};
