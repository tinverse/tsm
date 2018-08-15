#include <glog/logging.h>
#include <iostream>
#include <utility>

#include "EventQueue.h"
#include "State.h"
#include "StateMachine.h"
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
    smThread_ = std::thread(&StateMachine::execute, this);
}

void StateMachine::execute()
{
    while (!interrupt)
    {
        if (currentState_ == stopState_)
        {
            LOG(INFO) << "StateMachine Done Exiting... " << std::endl;
            interrupt = true;
            break;
        }
        else
        {
            LOG(INFO) << "Waiting for event";
            Event nextEvent = eventQueue_.nextEvent();

            LOG(INFO) << "Current State:" << currentState_->id
                      << " Event:" << nextEvent.id << std::endl;
            Transition* t;
            if ((t = table_.next(currentState_, nextEvent)) == nullptr)
            {
                if (parent_ != nullptr)
                {
                    parent_->getEventQueue().addFront(nextEvent);
                }
                else
                {
                    LOG(ERROR) << "Reached top level HSM. Cannot handle event";
                }
                continue;
            }

            // Perform the transition
            t->doTransition();

            currentState_ = t->toState;
            DLOG(INFO) << "Next State:" << currentState_->id << std::endl;

            // Now execute the current state
            currentState_->execute();

            if (currentState_ == stopState_) {
                break;
}
        }
    }
}
