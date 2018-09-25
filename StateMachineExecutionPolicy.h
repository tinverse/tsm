#pragma once

namespace tsm {
template<typename StateType, template<class> class ExecutionPolicy>
struct StateMachineExecutionPolicy
  : public StateType
  , public ExecutionPolicy<StateType>
{

    using ExecutionPolicy<StateType>::start;
    using ExecutionPolicy<StateType>::stop;

    StateMachineExecutionPolicy()
      : StateType()
      , ExecutionPolicy<StateType>(this)
    {}

    virtual ~StateMachineExecutionPolicy() = default;

    void onEntry() override
    {
        StateType::onEntry();
        start();
    }

    virtual void onExit() override
    {
        StateType::onExit();
        stop();
    }
};
} // namespace tsm
