#pragma once

#include <condition_variable>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template <typename Event> class EventQueue : private std::queue<Event>
{
    using std::queue<Event>::empty;
    using std::queue<Event>::front;
    using std::queue<Event>::pop;
    using std::queue<Event>::push;

public:
    // Block until you get an event
    const Event nextEvent()
    {
        // Note use of while instead of if below
        // See http://stackoverflow.com/questions/15278343/c11-thread-safe-queue
        while (empty())
        {
            // Wait until an event is available
            // There might be a bunch of threads blocked right here.
            std::cout << "Thread:" << std::this_thread::get_id()
                      << " grabbing eventLock\n";
            std::unique_lock<std::mutex> lock(eventLock);
            std::cout << "Thread:" << std::this_thread::get_id()
                      << " grabbed eventLock\n";
            cvEventAvailable.wait(lock);
            std::cout << "Thread:" << std::this_thread::get_id()
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
        std::cout << "Thread:" << std::this_thread::get_id()
                  << " grabbing eventQueueLock\n";
        std::lock_guard<std::mutex> lock(eventQueueLock);
        std::cout << "Thread:" << std::this_thread::get_id()
                  << " grabbed eventQueueLock\n";
        push(e);
        cvEventAvailable.notify_one();
        std::cout << "Thread:" << std::this_thread::get_id()
                  << " signaling event\n";
    }

private:
    std::mutex eventQueueLock;
    std::condition_variable cvEventAvailable;
    std::mutex eventLock;
};

