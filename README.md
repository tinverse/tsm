[![Build Status](https://travis-ci.org/tinverse/tsm.svg?branch=master)](https://travis-ci.org/tinverse/tsm)
[![codecov.io](http://codecov.io/github/tinverse/tsm/coverage.svg?branch=master)](http://codecov.io/github/tinverse/tsm?branch=master)
### What is tsm?
tsm is a flexible state machine framework with support for Hierarchical and Orthogonal State Machines.

### Features:
    * Hierarchical State machine (Unit tests and test examples welcome). 
    * Thread safe event queue. 
    * Ease of installation/distribution.
    * Ability to customize behavior by defining execution policies.

### Current Status
    * Thread-safe event queue. 
    * Hierarchical State Machine. 
    * No manual memory allocation.
    * 2 simultaneous Orthogonal State Machines.
    
### External Dependencies:
    Gflags, Glog, Gtest

### TODOs
    * support other ExecutionPolicy  classes.
      
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
For a primer on UML state machines, look [here][1]. 

######    Using tsm: [GarageDoorDef.h][2]

Create the state machine definition (HSMDef).
   ```
   struct GarageDoorDef : public StateMachineDef<GarageDoorDef>
   {
      GarageDoorDef(State* parent = nullptr)
        : StateMachineDef<GarageDoorDef>(parent)
        , name("Garage Door HSM")
        , doorOpen("Door Open")
        ...
      {
          // TransitionTable
          add(doorClosed, click_event, doorOpening);
          add(doorOpening, topSensor_event, doorOpen);
          ...
      }

      State* getStartState() const { return &doorClosed; }

      State* getStopState() const { return nullptr; }


      // States
      State doorOpen;
      State doorOpening;
      ...
      // Events
      Event click_event;
      ...
   };
   ```
Make sure that the getStartState and getStopState methods are overridden in the HSMDef.

Wrap the definition around a statemachine and then around an execution policy. Here we have two options. 

a. Create a state machine that executes in the context of the parent thread.
```
      SimpleStateMachine<GarageDoorDef> sm;
```

b. Create a state machine that processes incomming events in its own thread. 
```
      AsyncStateMachine<GarageDoorDef> sm;
```

Send events to the state machine by using sendEvent method provided by the policy.
```
    sm.sendEvent(sm.doorOpen);
```

If the state machine is running in parent thread context, invoke the `step` method to process the first event in the event queue. The AsyncStateMachine will immediately process an event on completion of prior event processing.
```
    sm.step();
```

[1]: https://en.wikipedia.org/wiki/UML_state_machine
[2]: https://github.com/tinverse/tsm/blob/master/test/GarageDoorDef.h

### Architecture 

Clients need include only the tsm.h file. 
The `StateMachine::getStartState` method should be overridden to reflect the correct starting state of your HSM.
    
The design uses CRTP to force Actions and Guards to be callbacks that are part of your HSM class. See the implementation of 

  ```
    template <typename DerivedHSM>
    struct StateMachine { 
    ...
  };
  ```
    
The AsyncStateMachine processes events in it's own thread. The processing of events is single threaded for OrthogonalHSMs (and HSMs). So when it is started using a call to `startSM`, the `StateMachine` will block on the call to `nextEvent` in the `execute` method. See tsm.h. The main advantage is that the only external interface to the StateMachine can be the EventQueue. Any "client" can asynchronously place an event in the event queue as long as they have a pointer to it. As soon as the StateMachine is done with its processing, it will pick up the first event in the queue and process it. This can be seen in the test/*.cpp files.
    
For testing the AsyncStateMachine, the AsyncExecWithObserver class is used with a special Observer class that blocks the parent thread until the AsyncStateMachine finishes event processing. The state machine thread then calls a notify method that releases the mutexblocking the parent thread.

States and Events generate unique Ids under the hood. This process is currently too simplistic. 
