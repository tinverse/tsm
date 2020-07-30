#pragma once
#include "Event.h"
#include "EventQueue.h"

#include <chrono>
#include <functional>
#include <thread>

namespace tsm {
///
/// This class implements a coarse timer that puts a "timer_event" in the event
/// queue at the expiration of a set time period. Its granularity is in the
/// milliseconds range.
///
struct ThreadSleepTimer
{
    explicit ThreadSleepTimer(std::chrono::milliseconds period,
                              std::function<void()>&& cb)
      : period_(period)
      , interrupt_(false)
      , cb_(cb)
    {}
    ThreadSleepTimer() = delete;
    ThreadSleepTimer(ThreadSleepTimer const&) = delete;
    ThreadSleepTimer(ThreadSleepTimer&&) = default;

    virtual ~ThreadSleepTimer()
    {
        stop();
        if (timerThread_.joinable()) {
            timerThread_.join();
        }
    }

    void start()
    {
        timerThread_ = std::thread([&]() {
            while (!interrupt_) {
                std::this_thread::sleep_for(period_);
                if (interrupt_) {
                    break;
                }
                this->cb_();
            }
        });
    }

    void stop() { interrupt_ = true; }

  private:
    std::chrono::duration<double, std::milli> period_;
    bool interrupt_;
    std::function<void()> cb_;
    std::thread timerThread_;
};

///
/// The policy for timed event processing. This Policy class works with a Timer
/// type. A callback from this policy is invoked from the Timer every time a
/// preset time period expires.
///
template<typename StateType, typename TimerType>
struct TimedExecutionPolicy : public StateType
{

    explicit TimedExecutionPolicy(std::chrono::milliseconds period)
      : StateType()
      , timer_(period, std::bind(&TimedExecutionPolicy::onTimerExpired, this))
    {}

    TimedExecutionPolicy(TimedExecutionPolicy const&) = delete;
    TimedExecutionPolicy(TimedExecutionPolicy&&) = delete;

    virtual ~TimedExecutionPolicy() = default;

    void onEntry(Event const& e) override
    {
        timer_.start();
        StateType::onEntry(e);
    }

    void onTimerExpired() { StateType::sendEvent(StateType::timer_event); }

    void onExit(Event const& e) override
    {
        DLOG(INFO) << "Exiting from Parent thread policy...";
        timer_.stop();
        StateType::onExit(e);
    }

  private:
    TimerType timer_;
};
} // namespace tsm
