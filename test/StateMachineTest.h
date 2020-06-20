#pragma once

#include "tsm.h"
using tsm::AsyncExecutionPolicy;
using tsm::Event;
using tsm::EventQueue;
using tsm::HsmExecutor;
using tsm::SingleThreadedExecutionPolicy;
using tsm::State;

namespace tsmtest {

// All state machine Hsms should derive from the state machine test class. The
// only difference from the base class is that the getCurrentState method has
// been overridden here in a particularly nasty way.
template<typename DerivedHsm>
struct StateMachineTest : public HsmExecutor<DerivedHsm>
{
    using type = StateMachineTest;
    using base_type = HsmExecutor<DerivedHsm>;

    StateMachineTest() = delete;

    StateMachineTest(std::string name,
                     EventQueue<Event>& eventQueue,
                     State* parent = nullptr)
      : HsmExecutor<DerivedHsm>(name, eventQueue, parent)
    {}

    virtual ~StateMachineTest() = default;

    void processEvent(Event nextEvent)
    {
        State* state = base_type::dispatch(this);
        state->execute(nextEvent);
    }
};
} // namespace tsmtest
