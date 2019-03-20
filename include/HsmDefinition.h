#pragma once

#include "FsmDefinition.h"
#include "State.h"

namespace tsm {
///
/// Interface for any Hierarchical SM.
///
struct IHsmDef
{
    IHsmDef() = delete;

    IHsmDef(IHsmDef* parent)
      : parent_(parent)
      , currentHsm_(nullptr)
    {}

    virtual ~IHsmDef() = default;

    IHsmDef* getCurrentHsm() { return currentHsm_; }

    void setCurrentHsm(IHsmDef* currentHsm) { currentHsm_ = currentHsm; }
    IHsmDef* getParent() const { return parent_; }

    virtual IHsmDef* dispatch() = 0;
    virtual void execute(Event const&) = 0;

    void setParent(IHsmDef* parent) { parent_ = parent; }

  protected:
    IHsmDef* parent_;
    IHsmDef* currentHsm_;
};

///
/// Captures 'structural' aspects of the state machine and behavior specific to
/// HsmDef. For e.g. the HsmDef can override the onEntry and onExit behaviors to
/// implement history preserving policy for specific events.
///
template<typename HsmDef>
struct HsmDefinition
  : public IHsmDef
  , public FsmDefinition<HsmDef>
{
    using Transition = typename FsmDefinition<HsmDef>::Transition;

    HsmDefinition() = delete;

    HsmDefinition(std::string const& name, IHsmDef* parent = nullptr)
      : IHsmDef(parent)
      , FsmDefinition<HsmDef>(name)
    {}

    virtual ~HsmDefinition() = default;
};
} // namespace tsm
