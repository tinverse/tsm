#include "Hsm.h"
#include "LoggingPolicy.h"
#include "PtAsyncExecutionPolicy.h"

#include "pt.h"

using tsm::Hsm;
using tsm::Event;

struct Switch : Hsm<Switch>
{
    Switch()
    {
        setStartState(&off);

        add(on, toggle, off, &Switch::onToggle);
        add(off, toggle, on, &Switch::onToggle);
    }

    uint32_t getToggles() const { return nToggles_; }
    void onToggle() { nToggles_++; std::cout << nToggles_ << std::endl; }

    State on, off;
    Event toggle;

  private:
    uint32_t nToggles_{};
};

template<typename StateType>
struct SwitchLoggingPolicy : public StateType
{
    void onEntry(Event const& e) override
    {
        if (StateType::getCurrentState()) {
            std::cout << "Entering State:" << StateType::getCurrentState()->id
                << " on" " Event:" << e.id << std::endl;
        } else {
            std::cout << "Entering State:" << StateType::getStartState()->id <<
                " on" " Event:" << e.id << std::endl;
        }
        StateType::onEntry(e);
    }

    void handle(Event const& nextEvent) override
    {
        std::cout << "Attempting transition from State: " <<
            StateType::getCurrentState()->id << " on Event:" << nextEvent.id <<
            std::endl;
        StateType::handle(nextEvent);
        std::cout << "Number of toggles:" << StateType::getToggles() << std::endl;
    }

    void onExit(Event const& e) override
    {
        std::cout << "Exiting State:" << StateType::getCurrentState()->id << " on"
            " Event:" << e.id << std::endl;
        StateType::onExit(e);
    }
};

using PtAsyncSwitch = tsm::PtAsyncExecutionPolicy<SwitchLoggingPolicy<Switch>>;
tsm::PtAsyncExecutionPolicy<SwitchLoggingPolicy<Switch>> mySwitch;
static
PT_THREAD(producer(struct pt* pt))
{
    PT_BEGIN(pt);
    for (auto i=0; i<10; ++i) {
        std::cout << "Adding Event:" << mySwitch.toggle.id << std::endl;
        mySwitch.sendEvent(mySwitch.toggle);
    }
    for (auto i=0; i<10; ++i) {
        std::cout << "Adding Event:" << mySwitch.toggle.id << std::endl;
        mySwitch.sendEvent(mySwitch.toggle);
    }
    PT_END(pt);
}

static
PT_THREAD(driver_thread(struct pt *pt))
{
    static struct pt pt_producer;
    static struct pt pt_consumer;
    PT_INIT(&pt_producer);
    PT_INIT(&pt_consumer);
    mySwitch.startSM();
    PT_BEGIN(pt);
    PT_WAIT_THREAD(pt, producer(&pt_producer) & mySwitch.step(&pt_consumer));
    /*
     * PT_WAIT_WHILE((pt), PT_SCHEDULE(thread))
     */
    mySwitch.stopSM();
  PT_END(pt);
}

#include <unistd.h>
int main() {
  struct pt driver_pt{};

  PT_INIT(&driver_pt);
  while(PT_SCHEDULE(driver_thread(&driver_pt))) {

    /*
     * When running this example on a multitasking system, we must
     * give other processes a chance to run too and therefore we call
     * usleep() resp. Sleep() here. On a dedicated embedded system,
     * we usually do not need to do this.
     */
    usleep(10);
  }
  return 0;

}
