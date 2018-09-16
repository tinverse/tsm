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
    using type = StateMachineTest;
    using base_type = StateMachineTest<DerivedHSM>;

    StateMachineTest() = delete;

    StateMachineTest(std::string name,
                     EventQueue<Event>& eventQueue,
                     State* parent = nullptr)
      : StateMachine<DerivedHSM>(name, eventQueue, parent)
    {}

    virtual ~StateMachineTest() = default;

    void processEvent(Event nextEvent)
    {
        State* state = base_type::dispatch(this);
        state->execute(nextEvent);
    }
};
} // namespace tsmtest
