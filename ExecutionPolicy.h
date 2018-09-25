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

    virtual void start() = 0;
    virtual void stop() = 0;

  protected:
    StateType* sm_;
};
} // namespace tsm
