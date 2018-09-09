#include <glog/logging.h>
#include <iostream>
#include <utility>

#include "EventQueue.h"
#include "State.h"
#include "Transition.h"
#include "tsm.h"

using tsm::Event;
using tsm::State;
using tsm::StateEventPair;
using tsm::StateMachine;

using Transition = tsm::StateMachine::Transition;

using std::shared_ptr;

bool
operator==(const StateEventPair& s1, const StateEventPair& s2)
{
    return (s1.first.get() == s2.first.get()) && (s1.second.id == s2.second.id);
}

shared_ptr<Transition>
StateMachine::StateTransitionTable::next(shared_ptr<State> fromState,
                                         Event onEvent)
{
    // Check if event in HSM
    StateEventPair pair(fromState, onEvent);
    auto it = find(pair);
    if (it != end()) {
        return it->second;
    }

    // print();
    std::ostringstream s;
    s << "No Transition:" << fromState->name << "\tonEvent:" << onEvent.id;
    LOG(ERROR) << s.str();
    return nullptr;
}

void
StateMachine::start()
{
    LOG(INFO) << "starting: " << name;
    currentState_ = startState_;
    // Only start a separate thread if you are the base Hsm
    if (!parent_) {
        smThread_ = std::thread(&StateMachine::execute, this);
    }

    LOG(INFO) << "started: " << name;
}

void
StateMachine::stop()
{
    // Stopping a HSM means stopping all of its sub HSMs
    LOG(INFO) << "stopping: " << name;
    for (auto& hsm : hsmSet_) {
        hsm->stop();
    }

    interrupt_ = true;

    if (!parent_) {
        eventQueue_.stop();
        smThread_.join();
    }
    LOG(INFO) << "stopped: " << name;
}

void
StateMachine::execute()
{
    while (!interrupt_) {
        if (currentState_ == stopState_) {
            LOG(INFO) << "StateMachine Done Exiting... ";
            interrupt_ = true;
            break;
        } else {
            LOG(INFO) << this->name << ": Waiting for event";

            Event nextEvent;
            try {
                nextEvent = eventQueue_.nextEvent();
            } catch (EventQueueInterruptedException const& e) {
                if (!interrupt_) {
                    throw e;
                }
                LOG(WARNING)
                  << this->name << ": Exiting event loop on interrupt";
                break;
            }

            LOG(INFO) << "Current State:" << currentState_->name
                      << " Event:" << nextEvent.id;

            shared_ptr<Transition> t = table_.next(currentState_, nextEvent);
            if (!t) {
                if (parent_) {
                    eventQueue_.addFront(nextEvent);
                    break;
                } else {
                    LOG(ERROR) << "Reached top level HSM. Cannot handle event";
                }
                // TODO (sriram): Maybe set to error state
                continue;
            }

            // This step exits the previous state (fromState.doExit()) and
            // enters the next tate. (toState.doEnter())
            t->doTransition();

            currentState_ = t->toState;

            LOG(INFO) << "Next State:" << currentState_->name;

            // Now execute the current state
            currentState_->execute();
        }
    }
}

shared_ptr<State> const&
StateMachine::getCurrentState() const
{
    LOG(INFO) << "GetState : " << this->name;
    return currentState_;
}

void
StateMachine::add(shared_ptr<State> fromState,
                  Event onEvent,
                  shared_ptr<State> toState)
{
    shared_ptr<Transition> t =
      std::make_shared<Transition>(fromState, onEvent, toState);

    addTransition(fromState, onEvent, t);
}

void
StateMachine::addTransition(shared_ptr<State> fromState,
                            Event onEvent,
                            shared_ptr<Transition> t)
{
    StateEventPair pair(fromState, onEvent);
    TransitionTableElement e(pair, t);
    table_.insert(e);

    // If HSM, add to hsm set.
    auto hsm = std::dynamic_pointer_cast<StateMachine>(fromState);
    if (hsm && !(hsm->getParent())) {
        hsm->setParent(this);
        hsmSet_.insert(hsm);
    }
}

void
StateMachine::StateTransitionTable::print()
{
    for (const auto& it : *this) {
        LOG(INFO) << it.first.first->name << "," << it.first.second.id << ":"
                  << it.second->toState->name << "\n";
    }
}
