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
    friend class StateMachine;

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
        std::unique_lock<std::mutex> lock(eventQueueLock);
        cvEventAvailable.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            throw EventQueueInterruptedException("Bailing from Event Queue");
        } else {
            // OK. Now we can modify the event queue.
            const Event e = front();
            LOG(INFO) << "Thread:" << std::this_thread::get_id()
                      << " Popping Event:" << e.id;
            pop_front();
            return e;
        }
    }

    void addEvent(const Event& e)
    {
        std::lock_guard<std::mutex> lock(eventQueueLock);
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " Adding Event:" << e.id << "\n";
        push_back(e);
        cvEventAvailable.notify_all();
    }

    void stop()
    {
        interrupt_ = true;
        cvEventAvailable.notify_all();
        // Log the events that are going to get dumped if the queue is not empty
    }

  private:
    void addFront(const Event& e)
    {
        std::lock_guard<std::mutex> lock(eventQueueLock);
        push_front(e);
        cvEventAvailable.notify_all();
    }

    std::mutex eventQueueLock;
    std::condition_variable cvEventAvailable;
    bool interrupt_;
};

} // namespace tsm
