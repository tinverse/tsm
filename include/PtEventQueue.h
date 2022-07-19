#include "pt.h"
#include "pt-sem.h"

#include <deque>

using std::deque;

namespace tsm {

template<typename Event>
struct EventQueueT : private deque<Event>
{
    using deque<Event>::empty;
    using deque<Event>::front;
    using deque<Event>::pop_front;
    using deque<Event>::push_back;
    using deque<Event>::push_front;
    using deque<Event>::size;

  public:
    EventQueueT(EventQueueT const&) = delete;
    EventQueueT(EventQueueT&&) = delete;
    EventQueueT operator=(EventQueueT const&) = delete;
    EventQueueT operator=(EventQueueT&&) = delete;

    EventQueueT()
    {
        PT_SEM_INIT(&empty_, 0);
        PT_SEM_INIT(&full_, QLIMIT);
    }

    ~EventQueueT() { stop(); }

    // Block until you get an event
    PT_THREAD(nextEvent(Event& e))
    {
        PT_BEGIN(&pt_consumer_);
        while (!interrupt_) {
            PT_SEM_WAIT(&pt_consumer_, &empty_);
            e = std::move(front());
            pop_front();
            PT_SEM_SIGNAL(&pt_consumer_, &full_);
        }
        PT_END(&pt_consumer_);
    }

    PT_THREAD(addEvent(Event const& e))
    {
        PT_BEGIN(&pt_producer_);

        while (!interrupt_) {
            PT_SEM_WAIT(&pt_producer_, &full_);
            push_back(e);
            PT_SEM_SIGNAL(&pt_producer_, &empty_);
        }
        PT_END(&pt_producer_);
    }

    void stop()
    {
        interrupt_ = true;
    }

    bool interrupted() { return interrupt_; }

  private:
    bool interrupt_{};
    struct pt pt_producer_{}, pt_consumer_{};
    struct pt_sem full_{}, empty_{};
    static const int QLIMIT = 32;
};

} // namespace tsm
