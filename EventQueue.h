#pragma once

#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <thread>

#include <glog/logging.h>

using std::deque;

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template <typename Event> class EventQueue : private deque<Event>
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
        : done_(false)
    {
    }

    ~EventQueue() { stop(); }

    // Block until you get an event
    const Event nextEvent()
    {
        // Note use of while instead of if below
        // See http://stackoverflow.com/questions/15278343/c11-thread-safe-queue
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " nextEvent grabbing eventQueueLock";
        std::unique_lock<std::mutex> lock(eventQueueLock);
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " nextEvent grabbed eventQueueLock\n";
        // Wait until an event is available
        // There might be a bunch of threads blocked right here.
        cvEventAvailable.wait(
            lock, [this] { return (!this->empty() || this->done_); });
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " event Available\n";
        if (done_)
        {
            return Event(5);
        }
        else
        {
            // OK. Now we can modify the event queue.
            const Event e = front();
            LOG(INFO) << "Popping Event:" << e.id << "\n";
            pop_front();
            return e;
        }
    }

    void addEvent(const Event& e)
    {
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " addEvent grabbing eventQueueLock\n";
        std::lock_guard<std::mutex> lock(eventQueueLock);
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " addEvent grabbed eventQueueLock\n";
        LOG(INFO) << "Adding Event:" << e.id << "\n";
        push_back(e);
        cvEventAvailable.notify_all();
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " signaling event\n";
    }

    void stop()
    {
        done_ = true;
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
    bool done_;
};
