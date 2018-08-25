#include <glog/logging.h>
#include <iostream>
#include <utility>

#include "EventQueue.h"
#include "State.h"
#include "tsm.h"
#include "Transition.h"

bool operator==(const StateEventPair& s1, const StateEventPair& s2)
{
    return (s1.first.get() == s2.first.get()) && (s1.second.id == s2.second.id);
}

Transition* StateTransitionTable::next(std::shared_ptr<State> fromState,
                                       Event onEvent)
{
    // Check if event in HSM
    StateEventPair pair(fromState, onEvent);
    auto it = find(pair);
    if (it != end())
    {
        return &it->second;
    }


        print();
        std::ostringstream s;
        s << "No Transition:" << fromState->name << "\t:" << onEvent.id
          << std::endl;
        LOG(ERROR) << s.str();
        return nullptr;

}

void StateMachine::start()
{
    currentState_ = startState_;
    currentStatePromise_.set_value(startState_);
    smThread_ = std::thread(&StateMachine::execute, this);
}

void StateMachine::execute()
{
    while (!interrupt_)
    {
        if (currentState_ == stopState_)
        {
            LOG(INFO) << "StateMachine Done Exiting... " << std::endl;
            interrupt_ = true;
            break;
        }
        else
        {
            LOG(INFO) << "Waiting for event";

            Event nextEvent;
            try {
                nextEvent = eventQueue_.nextEvent();
            } catch(EventQueueInterruptedException const & e) {
                if (!interrupt_) {
                    throw e;
                }
                break;
            }

            LOG(INFO) << "Current State:" << currentState_->name
                      << " Event:" << nextEvent.id << std::endl;
            Transition* t;
            if ((t = table_.next(currentState_, nextEvent)) == nullptr)
            {
                if (parent_ != nullptr)
                {
                    currentStatePromise_.set_value(currentState_);
                    parent_->addFront(nextEvent);
                }
                else
                {
                    LOG(ERROR) << "Reached top level HSM. Cannot handle event";
                }
                continue;
            }


            t->doTransition();

            currentState_ = t->toState;
            currentStatePromise_.set_value(currentState_);

            DLOG(INFO) << "Next State:" << currentState_->name << std::endl;

            // Now execute (enter?) the current state
            currentState_->execute();
        }
    }
}
