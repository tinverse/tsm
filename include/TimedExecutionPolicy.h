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
template<typename DurationType>
struct ThreadSleepTimer
{
    explicit ThreadSleepTimer(DurationType period, std::function<void()>&& cb)
      : period_(period)
      , interrupt_(false)
      , cb_(cb)
    {}

    ThreadSleepTimer(ThreadSleepTimer const&) = delete;
    ThreadSleepTimer operator=(ThreadSleepTimer const&) = delete;
    ThreadSleepTimer(ThreadSleepTimer&&) = delete;
    ThreadSleepTimer operator=(ThreadSleepTimer&&) = delete;

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
    DurationType period_;
    bool interrupt_;
    std::function<void()> cb_;
    std::thread timerThread_;
};

///
/// The policy for timed event processing. This Policy class works with a Timer
/// type. A callback from this policy is invoked from the Timer every time a
/// preset time period expires.
///
template<typename StateType,
         template<typename>
         class TimerType,
         typename DurationType>
struct TimedExecutionPolicy
  : public StateType
  , public TimerType<DurationType>
{
    using timer_type = TimerType<DurationType>;

    explicit TimedExecutionPolicy(DurationType period)
      : StateType()
      , timer_type(period,
                   std::bind(&TimedExecutionPolicy::onTimerExpired, this))
    {}

    void onEntry(Event const& e) override
    {
        timer_type::start();
        StateType::onEntry(e);
    }

    void onTimerExpired() { StateType::sendEvent(StateType::timer_event); }

    void onExit(Event const& e) override
    {
        LOG(INFO) << "Exiting from Parent thread policy...";
        timer_type::stop();
        StateType::onExit(e);
    }
};
} // namespace tsm
