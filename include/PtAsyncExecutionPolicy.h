#include <iostream>

#include "Event.h"
#include "PtEventQueue.h"

#include "pt.h"

namespace tsm {

struct PtThread
{
    PtThread()
    {
        PT_INIT(&pt_);
    }

    PT_THREAD(run()) {
        PT_BEGIN(&pt_);

        PT_END(&pt_);
    }
    private:
        struct pt pt_{};
};

struct PtSem
{
    explicit PtSem(int count)
    {
        PT_INIT(&pt_);
        PT_SEM_INIT(&pt_sem_, count);
    }

    PT_THREAD(run()) {
        PT_BEGIN(&pt_);
        PT_SEM_WAIT(&pt_, &pt_sem_);

        //PT_SEM_SIGNAL(&pt_, )
        PT_END(&pt_);
    }
    private:
        struct pt pt_{};
        struct pt_sem pt_sem_{};
};



template<typename StateType>
struct PtAsyncExecutionPolicy : public StateType
{
    using EventQueue = std::deque<Event>; //EventQueueT<Event>;

    void onEntry(Event const& e) override
    {
        StateType::onEntry(e);
    }

    void onExit(Event const& e) override
    {
        interrupt_ = true;

        StateType::onExit(e);
    }

    // Consumer
    PT_THREAD(step(struct pt* pt_p_))
    {
        PT_BEGIN(pt_p_);
        while (!interrupt_) {
            processEvent();
            PT_YIELD(pt_p_);
        }
        PT_END(pt_p_);
    };

    // Producer
    void sendEvent(Event const& event)
    {
        eventQueue_.push_back(event);
    }

    EventQueue eventQueue_;
  protected:
    bool interrupt_{};
    // struct pt pt_p_{};

    void processEvent()
    {
        if (!eventQueue_.empty()) {
            Event const& nextEvent = eventQueue_.front();
            eventQueue_.pop_front();
            StateType::dispatch(nextEvent);
        }
    }
};

#if 0
template<typename DurationType>
struct PtThreadSleepTimer
{
    explicit PtThreadSleepTimer(DurationType period, std::function<void()>&& cb)
      : period_(period)
      , cb_(cb)
      , pt_{}
    {}

    PtThreadSleepTimer(PtThreadSleepTimer const&) = delete;
    PtThreadSleepTimer operator=(PtThreadSleepTimer const&) = delete;
    PtThreadSleepTimer(PtThreadSleepTimer&&) = delete;
    PtThreadSleepTimer operator=(PtThreadSleepTimer&&) = delete;

    virtual ~PtThreadSleepTimer()
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

    PT_THREAD(threadfn())
    {
      PT_BEGIN(&pt_);
      while(!interrupt_) {
        std::this_thread::sleep_for(period_);
      }
      PT_END(&pt_);
    }
    void stop() { interrupt_ = true; }

  private:
    DurationType period_;
    bool interrupt_{};
    std::function<void()> cb_;
    std::thread timerThread_;
    struct timer timer_;
    struct pt pt_;
//struct timer timer;
};

#endif

} // namespace tsm


