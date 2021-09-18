#include "Event.h"
#include "tsm_log.h"

#include <iostream>

namespace tsm {
///
/// A simple and Generic LoggingPolicy. When inserted between say
/// AsyncExecutionPolicy and e.g. Hsm<Switch> to look like `using
/// AsyncLoggingSwitch = AsyncExecutionPolicy<LoggingPolicy<Switch>>>`,
/// entry, exit and transitions can be logged.
/// Pair with your favorite LOG macros.
///

template<typename StateType>
struct LoggingPolicy : public StateType
{
    void onEntry(Event const& e) override
    {
        if (StateType::getCurrentState()) {
            LOG(INFO) << "Entering State:" << StateType::getCurrentState()->id
                      << " on"
                         " Event:"
                      << e.id;
        } else {
            LOG(INFO) << "Entering State:" << StateType::getStartState()->id
                      << " on"
                         " Event:"
                      << e.id;
        }
        StateType::onEntry(e);
    }

    void handle(Event const& nextEvent) override
    {
        LOG(INFO) << "Attempting transition from State: "
                  << StateType::getCurrentState()->id
                  << " on Event:" << nextEvent.id;
        StateType::handle(nextEvent);
    }

    void onExit(Event const& e) override
    {
        LOG(INFO) << "Exiting State:" << StateType::getCurrentState()->id
                  << " on"
                     " Event:"
                  << e.id;
        StateType::onExit(e);
    }
};
} // namespace tsm
