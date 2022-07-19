#include "AsyncExecutionPolicy.h"
#include "Hsm.h"
#include "LoggingPolicy.h"
#include "tsm.h"

#include <catch2/catch.hpp>

using tsm::AsyncExecutionPolicy;
using tsm::Event;
using tsm::Hsm;
using tsm::LoggingPolicy;
using tsm::SingleThreadedHsm;

namespace tsmtest {
struct Switch : Hsm<Switch>
{
    Switch()
    {
        setStartState(&off);

        add(on, toggle, off, &Switch::onToggle);
        add(off, toggle, on, &Switch::onToggle);
    }

    uint32_t getToggles() const { return nToggles_; }
    void onToggle() { nToggles_++; LOG(INFO) << nToggles_; }

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
            LOG(INFO) << "Entering State:" << StateType::getCurrentState()->id
                << " on" " Event:" << e.id;
        } else {
            LOG(INFO) << "Entering State:" << StateType::getStartState()->id <<
                " on" " Event:" << e.id;
        }
        StateType::onEntry(e);
    }

    void handle(Event const& nextEvent) noexcept override
    {
        LOG(INFO) << "Attempting transition from State: " <<
            StateType::getCurrentState()->id << " on Event:" << nextEvent.id;
        StateType::handle(nextEvent);
        LOG(INFO) << "Number of toggles:" << StateType::getToggles();
    }

    void onExit(Event const& e) override
    {
        LOG(INFO) << "Exiting State:" << StateType::getCurrentState()->id << " on"
            " Event:" << e.id;
        StateType::onExit(e);
    }
};
} // namespace tsmtest

using tsmtest::Switch;
using tsmtest::SwitchLoggingPolicy;

// A simple Switch
TEST_CASE("TestSwitch - testSimpleSwitch")
{
    tsm::SingleThreadedHsm<Switch> mySwitch;
    constexpr const int TOGGLE_COUNT = 10;
    mySwitch.startSM();
    for (auto i=0; i < TOGGLE_COUNT; ++i) {
        mySwitch.sendEvent(mySwitch.toggle);
        mySwitch.step();
        if (i%2 == 0) {
            REQUIRE(mySwitch.getCurrentState() == &mySwitch.on);
        } else {
            REQUIRE(mySwitch.getCurrentState() == &mySwitch.off);
        }
    }
}


// This will exercise the code in the logging policy, but we will not be able to
// verify what's getting logged without a lot of effort.
using AsyncLoggingSwitch = AsyncExecutionPolicy<LoggingPolicy<Switch>>;
TEST_CASE("TestSwitch - testSwitchGenericLogging")
{
    AsyncLoggingSwitch mySwitch;
    constexpr const int TOGGLE_COUNT = 10;
    mySwitch.startSM();
    for (auto i=0; i < TOGGLE_COUNT; ++i) {
        mySwitch.sendEvent(mySwitch.toggle);
    }
}

// More customized logging can also be implemented as shown above with the
// `SwitchLoggingPolicy`.
using AsyncCustomLoggingSwitch = AsyncExecutionPolicy<SwitchLoggingPolicy<Switch>>;
TEST_CASE("TestSwitch - testSwitchCustomLogging")
{
    AsyncCustomLoggingSwitch mySwitch;
    constexpr const int TOGGLE_COUNT = 10;
    mySwitch.startSM();
    for (auto i=0; i < TOGGLE_COUNT; ++i) {
        mySwitch.sendEvent(mySwitch.toggle);
    }

}
