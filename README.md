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
    * Support other ExecutionPolicy  classes.
    * States and Events generate unique Ids under the hood. This process is currently too simplistic. 
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

######    Using tsm

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

c. Send events to the state machine by using sendEvent method provided by the policy.
```
    sm.sendEvent(sm.doorOpen);
```

d. If the state machine is running in parent thread context, invoke the `step` method to process the first event in the event queue. The `AsyncStateMachine` will immediately process an event on completion of prior event processing.

For a complete example see  [GarageDoorDef.h][2] and [GarageDoorDef.cpp][3]. Also look at [CdPlayerHSM.h][4] [CdPlayerHSM.cpp][5]

```
    sm.step();
```

[1]: https://en.wikipedia.org/wiki/UML_state_machine
[2]: https://github.com/tinverse/tsm/blob/master/test/GarageDoorDef.h
[3]: https://github.com/tinverse/tsm/blob/master/test/GarageDoorDef.cpp
[4]: https://github.com/tinverse/tsm/blob/master/test/CdPlayerHSM.h
[5]: https://github.com/tinverse/tsm/blob/master/test/CdPlayerHSM.cpp

### Architecture 

#### Policy based design
Classes have been partitioned across policies so they can be mixed and matched for code reuse. The current architecture supports state machines with the following characterestics.
```
a. Hierarchical 
b. Asynchronous
c. Parallel/Orthogonal
d. History Preserving
```

Other Policy classes can be implemented for distributed event processing. The existing mechanism can also be extended to incorporate custom behavior such as writing state transitions to disk. For e.g. see `struct AsyncExecWithObserver` in `AsyncExecutionPolicy.h` 

Clients need to include tsm.h file and link against the libtsm library.

The abstract base class `IHsmDef` forces all Hierarchical State Machines to override the `getStartState` and `getStopState` methods. 

The design uses CRTP to force Actions and Guards to be callbacks that are part of your HSM class. See the implementation of 

```
    template <typename HsmDef>
    struct StateMachineDef : public IHsmDef { 
    ...
  };

```
#### HSMDef
Place holder for your own application specific sate machine definition. For e.g. you might create your own HSM called `CdPlayerHSM`. This HSM should inherit from `StateMachineDef` using CRTP.

```
    struct CdPlayerHSM : public StateMachineDef<CdPlayerHSM> { 
    ...
  };

```

#### StateMachineDef
StateMachineDef is a generic that takes a user defined HSM as a template type parameter. Inheriting from `StateMachineDef` gives your state machine type - in this case the `CdPlayerHSM`, a `StateTransitionTable` class and associated boiler plate functionality for performing state transitions. 

#### StateMachine
The `StateMachine` generic implements functionality that is common to all HSMs. It provides methods `startSM` and `stopSM` that are self explanatory. The `dispatch` method forwards the event to be processed to the innermost HSM. In one of he policy classes, `execute` is then invoked on that (innermost) HSM. See `AsyncExecutionPolicy::processEvent`.

#### Policy Classes
Policy classes like `AsyncExecutionPolicy` and `ParentThreadExecutionPolicy` are mixins that operate on `StateType`s i.e. any type with `onEntry` and `onExit` methods that can be overridden. Clients will typically interact with a Policy class at the bottom of the inheritance hierarcy. By convention, these Policy classes also provide a `sendEvent` method as a public interface to the state machine. The `ParentThreadExecutionPolicy` class also provides a `step` method for clients to initiate event processing. 

The `AsyncStateMachine` processes events in it's own thread. The processing of events is single threaded within all HSMs. So when a HSM is started using a call to `startSM`, the `StateMachine` will block on the call to `nextEvent` in the `execute` method. See tsm.h. The main advantage is that the only external interface to the StateMachine can be the EventQueue. Any "client" can asynchronously place an event in the event queue as long as they have a pointer to it. As soon as the StateMachine is done with its processing, it will pick up the first event in the queue and process it. This can be seen in the test/*.cpp files.

#### Putting it all together
Create your own state machine definition that derives from `StateMachineDef`. The HSM hierarchy, its states (and sub-HSMs if any), events, actions and guards are all specified and defined in the definition. The relationships between HSMs and the state transition table is also specified here. All code related to your HSM lives here.
Then choose a policy class for your state machine. Create your own state machine type by wrapping the policy class around the `StateMachine` generic. The unit test provided below is illustrative. 

```
///
/// GarageDoorDef is the state machine definition. It has knowledge of the HSM
/// hierarchy, its states, events and sub-HSMs if any. The relationships between
/// HSMs (parentHsm_) is also setup here. Mix the Async observer and the
/// GarageDoor StateMachine to get an asynchronous garage door state machine
/// that notifies a listener at the end of processing each event.
///
using GarageDoorHSMSeparateThread =
  AsyncBlockingObserver<StateMachine<GarageDoorDef>>;

TEST_F(TestGarageDoorSM, testGarageDoorSeparateThreadPolicy)
{
    auto sm = std::make_shared<GarageDoorHSMSeparateThread>();
    auto smDef = std::static_pointer_cast<GarageDoorDef>(sm);

    sm->startSM();

    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorClosed);

    sm->sendEvent(smDef->click_event);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpening);

    sm->sendEvent(smDef->topSensor_event);
    sm->wait();
    ASSERT_EQ(sm->getCurrentState(), &smDef->doorOpen);

    sm->stopSM();
}
```

#### Testing
For testing the `AsyncStateMachine`, the `AsyncExecWithObserver` class is used with a special `Observer` class that blocks the parent thread until the `AsyncStateMachine` finishes event processing. The state machine thread then calls a `notify` method that releases the mutex blocking the parent thread.
