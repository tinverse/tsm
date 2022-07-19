#pragma once

#include "tsm_log.h"
#ifndef __arm__
#include <condition_variable>
#include <deque>
#include <iostream>
#include <mutex>
#include <string>

using std::deque;

namespace tsm {

// A thread safe event queue. Any thread can call addEvent if it has a pointer
// to the event queue. The call to nextEvent is a blocking call
template<typename Event, typename LockType>
struct EventQueueT : private deque<Event>
{
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

  public:
    EventQueueT() = default;
    EventQueueT(EventQueueT const&) = delete;
    EventQueueT(EventQueueT&&) = delete;
    EventQueueT operator=(EventQueueT const&) = delete;
    EventQueueT operator=(EventQueueT&&) = delete;

    ~EventQueueT() { stop(); }

    // Block until you get an event
    Event nextEvent()
    {
        std::unique_lock<LockType> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            return Event();
        }
        const Event e = std::move(front());
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Popping Event:" << e.id;
        pop_front();
        return e;
    }

    void addEvent(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Adding Event:" << e.id;
        push_back(e);
        cvEventAvailable_.notify_all();
    }

    void stop()
    {
        interrupt_ = true;
        cvEventAvailable_.notify_all();
        // Log the events that are going to get dumped if the queue is not
        // empty
    }

    bool interrupted() { return interrupt_; }

    void addFront(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        push_front(e);
        cvEventAvailable_.notify_all();
    }

  private:
    LockType eventQueueMutex_;
    std::condition_variable_any cvEventAvailable_;
    bool interrupt_{};
};

template<typename Event>
using EventQueue = EventQueueT<Event, std::mutex>;
#else
// Assume FreeRTOS for now
#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define INCLUDE_eTaskGetState 1
#define configUSE_TIME_SLICING 1

#include "FreeRTOS.h"
#include "task.h"
#include "event_groups.h"
#include "semphr.h"

constexpr defer_lock_t defer_lock{};
inline constexpr defer_lock_t defer_lock{};

struct mutex {
    using native_handle_type = SemaphoreHandle_t;
    mutex() noexcept
        : mutex_(xSemaphoreCreateMutex())
    {
        xSemaphoreGive(mutex_);
    }
    mutex(std::defer_lock_t /* unused */) noexcept
        : mutex_(xSemaphoreCreateMutex())
    {}

    mutex(mutex const&) = delete;
    mutex operator=(mutex const&) = delete;

    mutex(mutex&&) = default;
    mutex operator=(mutex&&) = default;

    ~mutex() {
        vSemaphoreDelete(mutex_);
    }

    void lock() {
        xSemaphoreTake(mutex_);
    }

    void unlock() {
        xSemaphoreGive(mutex_);
    }

    native_handle_type native_handle() {
        return mutex_;
    }

    SemaphoreHandle_t mutex_;
};

template <class Mutex>
struct unique_lock {
    using mutex_type = Mutex;
    unique_lock() = delete;
    explicit unique_lock(Mutex& mutex)
        : mutex_(mutex) {
            mutex_.lock();
        }
    unique_lock(Mutex& mutex, defer_lock_t /* unused */)
        : mutex_(mutex) {}

    unique_lock(unique_lock const&) = delete;
    unique_lock operator=(unique_lock const&) = delete;

    unique_lock(unique_lock&&) = default;
    unique_lock operator=(unique_lock&&) = default;

    void lock() {
        mutex.lock();
    }

    void unlock() {
        mutex.unlock();
    }

    Mutex& mutex_;
};

struct condition_variable_any {
    condition_variable_any()
        : evtGrp_() {
    }
    condition_variable_any(condition_variable_any const&) = delete;
    condition_variable_any operator=(condition_variable_any const&) = delete;
    condition_variable_any(condition_variable_any&&) = delete;
    condition_variable_any operator=(condition_variable_any&&) = delete;
    static constexpr int NOTIFY = 0x01;
    static constexpr int EXIT = 0x02;
    void notify_all() {
        xEventGroupSetBits(evtGrp_, NOTIFY);
    }

    template <typename Lock>
        wait(Lock& lock) {
            taskDISABLE_INTERRUPTS();
            lock.unlock();
            xEventGroupWaitBits(evtGrp_, NOTIFY | EXIT, pdTRUE, pdFALSE)

        }
    EventGroupHandle_t evtGrp_;
};

template<typename Event, typename LockType>
struct EventQueueT : private deque<Event>
{
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

  public:
    EventQueueT() = default;
    EventQueueT(EventQueueT const&) = delete;
    EventQueueT(EventQueueT&&) = delete;
    EventQueueT operator=(EventQueueT const&) = delete;
    EventQueueT operator=(EventQueueT&&) = delete;

    ~EventQueueT() { stop(); }

    // Block until you get an event
    Event nextEvent()
    {
        std::unique_lock<LockType> lock(eventQueueMutex_);
        cvEventAvailable_.wait(
          lock, [this] { return (!this->empty() || this->interrupt_); });
        if (interrupt_) {
            return Event();
        }
        const Event e = std::move(front());
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Popping Event:" << e.id;
        pop_front();
        return e;
    }

    void addEvent(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        // LOG(INFO) << "Thread:" << std::this_thread::get_id()
        //          << " Adding Event:" << e.id;
        push_back(e);
        cvEventAvailable_.notify_all();
    }

    void stop()
    {
        interrupt_ = true;
        cvEventAvailable_.notify_all();
        // Log the events that are going to get dumped if the queue is not
        // empty
    }

    bool interrupted() { return interrupt_; }

    void addFront(Event const& e)
    {
        std::lock_guard<LockType> lock(eventQueueMutex_);
        push_front(e);
        cvEventAvailable_.notify_all();
    }

  private:
    LockType eventQueueMutex_;
    std::condition_variable_any cvEventAvailable_;
    bool interrupt_{};
}
#endif
} // namespace tsm
