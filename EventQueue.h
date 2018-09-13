#pragma once

#include <condition_variable>
#include <deque>
#include <glog/logging.h>
#include <iostream>
#include <mutex>
#include <numeric>
#include <string>
#include <thread>

using std::deque;

namespace tsm {

struct EventQueueInterruptedException : public std::runtime_error
{
    explicit EventQueueInterruptedException(const std::string& what_arg)
      : std::runtime_error(what_arg)
    {}
    explicit EventQueueInterruptedException(const char* what_arg)
      : std::runtime_error(what_arg)
    {}
};

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template<typename Event>
class EventQueue : private deque<Event>
{
  public:
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

    EventQueue()
      : interrupt_(false)
    {}

    ~EventQueue() { stop(); }

    // Block until you get an event
    const Event nextEvent()
    {
        std::unique_lock<std::mutex> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            throw EventQueueInterruptedException("Bailing from Event Queue");
        } else {
            const Event e = front();
            DLOG(INFO) << "Thread:" << std::this_thread::get_id()
                       << " Popping Event:" << e.id;
            pop_front();
            return e;
        }
    }

    void addEvent(const Event& e)
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex_);
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

    void addFront(const Event& e)
    {
        std::lock_guard<std::mutex> lock(eventQueueMutex_);
        push_front(e);
        cvEventAvailable_.notify_all();
    }

    auto& getConditionVariable() { return cvEventAvailable_; }

    auto& getEventQueueMutex() { return eventQueueMutex_; }

  private:
    std::mutex eventQueueMutex_;
    std::condition_variable cvEventAvailable_;
    bool interrupt_;
};

} // namespace tsm
