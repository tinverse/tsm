#pragma once

namespace tsm {
template<typename StateType, template<class> class ExecutionPolicy>
struct StateMachineWithExecutionPolicy
  : public StateType
  , public ExecutionPolicy<StateType>
{
    using ExecutionPolicy<StateType>::entry;
    using ExecutionPolicy<StateType>::exit;

    StateMachineWithExecutionPolicy()
      : StateType()
      , ExecutionPolicy<StateType>(this)
    {}

    virtual ~StateMachineWithExecutionPolicy() = default;

    void onEntry(Event const& e) override
    {
        StateType::onEntry(e);
        entry();
    }

    void onExit(Event const& e) override
    {
        exit();
        StateType::onExit(e);
    }
};
} // namespace tsm
