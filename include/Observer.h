#pragma once
#include "tsm_log.h"

#include <condition_variable>
#include <functional>
#include <mutex>
#include <vector>

namespace tsm {
///
/// A simple observer class. The notify method will be invoked by an
/// AsyncExecWithObserver state machine after event processing. This observer
/// also implements a blocking wait so that test methods can invoke the wait
/// method to synchronize with the event processing.
///
struct BlockingObserver
{
    BlockingObserver()
      : notified_(false)
    {}

    void notify()
    {
        std::unique_lock<std::mutex> lock(smBusyMutex_);
        notified_ = true;
        DLOG(INFO) << "Notify.....";
        cv_.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(smBusyMutex_);
        DLOG(INFO) << "Wait.....";
        cv_.wait(lock, [this] { return this->notified_ == true; });
        notified_ = false;
    }

  private:
    std::mutex smBusyMutex_;
    std::condition_variable cv_;
    bool notified_;
};

struct CallbackObserver
{
    void addCallback(std::function<void()>&& cb)
    {
        if (cb != nullptr) {
            cbs_.push_back(cb);
        }
    }

    void notify()
    {
        for (auto const& cb : cbs_) {
            cb();
        }
    }

  private:
    std::vector<std::function<void()>> cbs_;
};

} // namespace tsm
