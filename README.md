[![Build Status](https://travis-ci.org/tinverse/tsm.svg?branch=master)](https://travis-ci.org/tinverse/tsm)
[![codecov.io](http://codecov.io/github/tinverse/tsm/coverage.svg?branch=master)](http://codecov.io/github/tinverse/tsm?branch=master)
### Goals:
    * Hierarchical State machine (Unit tests and other contributions welcome).
    * Thread safe event queue.
    * C++14 - no manual memory management, C++ std libraries only.
    * Ease of installation/ distribution.

### Current Status
    * Thread-safe event queue.
    * Hierarchical State Machine.
    * No manual memory allocation.
    * 2 simultaneous Orthogonal State Machines.

### External Dependencies:
    GFlags, GLog, Gtest

### TODOs
    * More unit tests needed.
    * Rewrite base StateMachineTest's getCurrentState override.
    * Have StateMachine optionally execute in calling thread (non-blocking).

### Build:
Install cmake (sudo apt install cmake) or download on windows and add cmake to system path. Read the CMakeLists.txt to get a good feel for it. The cmake/superbuild/ folder contains the cmake files that download and install the external dependencies.

```
git clone https://github.com/tinverse/tsm.git
cd tsm
mkdir build
cd build
cmake -DBUILD_DEPENDENCIES=ON ..
make -j4 #or however many cores are at your disposal
cmake -DBUILD_DEPENDENCIES=OFF ..
make -j4
./tsm_tests
```

### Documentation
For a primer on UML state machines, look [here][1]. Here is an example below.

######    [GarageDoorSM.h][2]

   ```c++
   struct GarageDoorSM : public StateMachineTest<GarageDoorSM>
   {
      GarageDoorSM(std::string name, EventQueue<Event>& eventQueue)
        : StateMachineTest<GarageDoorSM>(name, eventQueue)
        , doorOpen(std::make_shared<State>("Door Open"))
        ...
      {
          // TransitionTable
          add(doorClosed, click_event, doorOpening);
          add(doorOpening, topSensor_event, doorOpen);
          ...
      }

      shared_ptr<State> getStartState() const override { return doorClosed; }

      // States
      shared_ptr<State> doorOpen;
      shared_ptr<State> doorOpening;
      ...
      // Events
      Event click_event;
      ...
   };
   ```

[1]: https://en.wikipedia.org/wiki/UML_state_machine
[2]: https://github.com/tinverse/tsm/blob/master/test/GarageDoorSM.h
### Quirks
Please be aware of these quirks in the design.

The `StateMachine::getStartState` method should be overridden to reflect the correct starting state of your HSM.

The design uses CRTP to force Actions and Guards to be callbacks that are part of your Concrete HSM class. See the implementation of

    ```c++
    template <typename DerivedHSM>
    struct StateMachine {
    ...
    };
    ```

The StateMachine processes events in it's own thread. The processing of events is single threaded for OrthogonalHSMs (and HSMs). So when it is started using a call to `startHSM`, the `StateMachine` will block on the call to `nextEvent` in the `execute` method. See tsm.h. The main advantage is that the only external interface to the StateMachine can be the EventQueue. Any "client" can asynchronously place an event in the event queue as long as they have a pointer to it. As soon as the StateMachine is done with its processing, it will pick up the first event in the queue and process it. This can be seen in the test/*.cpp files.

After placing the event in the event queue, the StateMachineTest class sleeps for a couple of milliseconds for the state machine to pick up the event, process it and update the current state. This must be synchronized better. Way better.

States and Events generate unique Ids under the hood. This process is currently too simplistic.
