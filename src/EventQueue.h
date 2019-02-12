#pragma once

#include <glog/logging.h>

#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>

using std::deque;

namespace tsm {

struct dummy_mutex
{
    dummy_mutex() = default;
    ~dummy_mutex() = default;

    dummy_mutex(dummy_mutex&) = delete;
    dummy_mutex(dummy_mutex const&) = delete;
    dummy_mutex(dummy_mutex&&) = delete;

    void lock() {}
    void unlock() {}
    bool try_lock() { return true; }
};

struct EventQueueInterruptedException : public std::runtime_error
{
    explicit EventQueueInterruptedException(const std::string& what_arg)
      : std::runtime_error(what_arg)
    {}
};

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template<typename Event, typename LockType>
class EventQueueT : private deque<Event>
{
  public:
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

    EventQueueT()
      : interrupt_(false)
    {}

    ~EventQueueT() { stop(); }

    // Block until you get an event
    const Event nextEvent()
    {
        std::unique_lock<LockType> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            throw EventQueueInterruptedException("Bailing from Event Queue");
        } else {
            const Event e = std::move(front());
            DLOG(INFO) << "Thread:" << std::this_thread::get_id()
                       << " Popping Event:" << e.id;
            pop_front();
            return std::move(e);
        }
    }

    void addEvent(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        DLOG(INFO) << "Thread:" << std::this_thread::get_id()
                   << " Adding Event:" << e.id << "\n";
        push_back(e);
        cvEventAvailable_.notify_all();
    }

    void stop()
    {
        interrupt_ = true;
        cvEventAvailable_.notify_all();
        // Log the events that are going to get dumped if the queue is not empty
    }

    void addFront(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        push_front(e);
        cvEventAvailable_.notify_all();
    }

  private:
    LockType eventQueueMutex_;
    std::condition_variable_any cvEventAvailable_;
    bool interrupt_;
};

template<typename Event>
using SimpleEventQueue = EventQueueT<Event, dummy_mutex>;

template<typename Event>
using EventQueue = EventQueueT<Event, std::mutex>;

} // namespace tsm
