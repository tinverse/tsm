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
        cv_.notify_all();
    }

    void wait()
    {
        std::unique_lock<std::mutex> lock(smBusyMutex_);
        cv_.wait(lock, [this] { return this->notified_ == true; });
        notified_ = false;
    }

  private:
    std::mutex smBusyMutex_;
    std::condition_variable cv_;
    bool notified_;
};
}
