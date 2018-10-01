#pragma once

namespace tsm {
template<typename StateType>
struct ExecutionPolicy
{
    ExecutionPolicy() = delete;
    ExecutionPolicy(StateType* sm)
      : sm_(sm)
    {}

    virtual ~ExecutionPolicy() = default;

    virtual void entry() = 0;
    virtual void exit() = 0;

  protected:
    StateType* sm_;
};
} // namespace tsm
