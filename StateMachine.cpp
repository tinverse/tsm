#include <glog/logging.h>
#include <iostream>

#include "EventQueue.h"
#include "State.h"
#include "StateMachine.h"
#include "Transition.h"


void StateMachine::start()
{
    smThread_ = std::thread(&StateMachine::execute, this);
}

void StateMachine::execute(void)
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

            Transition& t = table_.next(currentState_, nextEvent);

            // Perform the transition
            t.fromState->OnExit();
            t.execute();
            t.toState->OnEntry();

            currentState_ = t.toState;
            DLOG(INFO) << "Next State:" << currentState_->id << std::endl;

            // Now execute the current state
            currentState_->execute();

            if (currentState_ == stopState_) break;
        }
    }
}
