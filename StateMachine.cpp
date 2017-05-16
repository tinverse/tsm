#include <glog/logging.h>
#include <iostream>

#include "EventQueue.h"
#include "State.h"
#include "StateMachine.h"
#include "Transition.h"


void StateMachine::start()
{
    running   = true;
    smThread_ = std::thread(&StateMachine::execute, this);
}

void StateMachine::execute(void)
{
    while (running)
    {
        if (currentState_ == stopState_)
        {
            DLOG(INFO) << "Done... " << std::endl;
            running = false;
            break;
        }
        else
        {
            Event nextEvent = eventQueue_.nextEvent();

            DLOG(INFO) << "Current State:" << currentState_->id
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
        }
    }
}
