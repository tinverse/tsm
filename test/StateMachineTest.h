#pragma once

#include "tsm.h"
using tsm::Event;
using tsm::EventQueue;
using tsm::State;
using tsm::StateMachine;

namespace tsmtest {

// All state machine HSMs should derive from the state machine test class. The
// only difference from the base class is that the getCurrentState method has
// been overridden here in a particularly nasty way.
template<typename DerivedHSM>
struct StateMachineTest : public StateMachine<DerivedHSM>
{
    StateMachineTest() = delete;

    StateMachineTest(std::string name,
                     EventQueue<Event>& eventQueue,
                     State* parent = nullptr)
      : StateMachine<DerivedHSM>(name, eventQueue, parent)
      , eventQueue_(eventQueue)
      , cvEventAvailable_(eventQueue.getConditionVariable())
      , eventQueueMutex_(eventQueue.getEventQueueMutex())
    {}

    virtual ~StateMachineTest() = default;

    std::shared_ptr<State> const& getCurrentState() const override
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        return StateMachine<DerivedHSM>::currentState_;
    }
    EventQueue<Event>& eventQueue_;
    std::condition_variable& cvEventAvailable_;
    std::mutex& eventQueueMutex_;
};
} // namespace tsmtest
