#include "StateMachine.h"
#include <chrono>

// Simple test garageDoorStateMachine
int main()
{

    // States
    auto doorOpen    = std::make_shared<State>(10);
    auto doorOpening = std::make_shared<State>(15);
    auto doorClosed  = std::make_shared<State>(20);
    auto doorClosing = std::make_shared<State>(25);

    // Events
    Event close_event(1);
    Event bottomSensor_event(1);
    Event topSensor_event(1);
    Event open_event(2);

    EventQueue<Event> garageDoorEventQueue;

    // TransitionTable
    StateTransitionTable garageDoorTransitions;

    // Transitions
    Transition openTransition(doorClosed, open_event, doorOpening);
    Transition openedTransition(doorOpening, topSensor_event, doorOpen);
    Transition closeTransition(doorOpen, close_event, doorClosing);
    Transition closedTransition(doorClosing, bottomSensor_event, doorClosed);

    // Add Transitions to the table
    garageDoorTransitions.add(openTransition);
    garageDoorTransitions.add(openedTransition);
    garageDoorTransitions.add(closeTransition);
    garageDoorTransitions.add(closedTransition);

    // Add the open event
    garageDoorEventQueue.addEvent(open_event);

    StateMachine garageDoorStateMachine(
        doorClosed, doorOpen, garageDoorEventQueue, garageDoorTransitions);

    garageDoorStateMachine.start();

    while (garageDoorStateMachine.running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        garageDoorEventQueue.addEvent(topSensor_event);
    }

    garageDoorStateMachine.stop();


    return 0;
}
