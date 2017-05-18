#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>

#include <glog/logging.h>

using std::queue;

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template <typename Event> class EventQueue : private queue<Event>
{
public:
    using queue<Event>::empty;
    using queue<Event>::front;
    using queue<Event>::pop;
    using queue<Event>::push;
    using queue<Event>::size;

    // Block until you get an event
    const Event nextEvent()
    {
        // Note use of while instead of if below
        // See http://stackoverflow.com/questions/15278343/c11-thread-safe-queue
        while (empty())
        {
            // Wait until an event is available
            // There might be a bunch of threads blocked right here.
            LOG(INFO) << "Thread:" << std::this_thread::get_id()
                                   << " grabbing eventLock";
            std::unique_lock<std::mutex> lock(eventLock);
            LOG(INFO) << "Thread:" << std::this_thread::get_id()
                                   << " grabbed eventLock\n";
            cvEventAvailable.wait(lock);
            LOG(INFO) << "Thread:" << std::this_thread::get_id()
                                   << " event Available\n";
        }

        // Multiple threads can get to this point. However, only one thread can
        // process the event
        {
            std::lock_guard<std::mutex> l(eventQueueLock);
            // OK. Now we can modify the event queue.
            const Event e = front();
            pop();
            return e;
        }
    }

    void addEvent(const Event& e)
    {
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " grabbing eventQueueLock\n";
        std::lock_guard<std::mutex> lock(eventQueueLock);
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " grabbed eventQueueLock\n";
        push(e);
        cvEventAvailable.notify_all();
        LOG(INFO) << "Thread:" << std::this_thread::get_id()
                  << " signaling event\n";
    }

private:
    std::mutex eventQueueLock;
    std::condition_variable cvEventAvailable;
    std::mutex eventLock;
};
