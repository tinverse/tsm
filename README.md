[![Build Status](https://travis-ci.org/tinverse/tsm.svg?branch=master)](https://travis-ci.org/tinverse/tsm)
[![codecov.io](http://codecov.io/github/tinverse/tsm/coverage.svg?branch=master)](http://codecov.io/github/tinverse/tsm?branch=master)
[![Coverity Scan](https://scan.coverity.com/projects/17873/badge.svg)](https://scan.coverity.com/projects/tinverse-tsm)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

The [C++ API documentation](https://tinverse.github.io/tsm/index.html) has an overview of usage and architecture and a discussion of drawbacks.

### What is tsm?
tsm is a state machine framework with support for Hierarchical and Orthogonal State Machines.

### Features:
    * Hierarchical State machine.
    * Thread safe event queue.
    * Single Threaded or Asynchronous execution.
    * Timer driven execution policy.
    * Ease of installation and integration - Header only, CMake and Nix support.
    * Policy Based - Ability to customize behavior by defining execution policies.

### Quick Overview
#### A Finite State Machine

```cpp
struct Switch : Hsm<Switch>
{
    Switch()
        : Hsm<Switch>("A Simple Switch")
        , on("on")
        , off("off")
    {
        add(on, toggle, off);
        add(off, toggle, on);
    }

    State* getStartState() const { return &on; }
    State* getStopState() const { return nullptr; }

    State on;
    State off;

    Event toggle;
}
```

So there you have it. Unfortunately, with tsm (this framework), it doesn't get any simpler than this. Trade-offs. We are not done yet. There is one more thing. How do you expect clients to interact with this state machine? There are two obvious choices.
    * Single Threaded Execution Policy

```cpp
using SwitchFsm = SingleThreadedHsm<Switch>;
int main() {
    SwitchFsm s;
    s.startSM();
    s.sendEvent(s.toggle);
    s.step();
    // ...
    s.stopSM();
}
```

    * Asynchronous Execution Policy

```cpp
using SwitchFsm = AsynchronousHsm<Switch>;
int main() {
    SwitchFsm s;
    s.startSM();
    s.sendEvent(s.toggle);
    // ...
    s.stopSM();
}
```

`s.step()` is missing in the AsynchronousHsm. The EventQueue blocks waiting for the next event to arrive and processes it as soon as it arrives.

#### A Hierarchical State Machine
Assume you are trying to model a `CdPlayer` as a state machine.

```cpp
struct CdPlayer : public Hsm<CdPlayer>
{
    // Actions
    void PlaySong()
    {
        LOG(INFO) << "Play Song";
    }

    // Guards
    bool PlaySongGuard()
    {
        DLOG(INFO) << "Play Song Guard";
        return true;
    }
    // States
    State Stopped;
    <b>PlayingHsm Playing;</b>
    State Paused;
    State Empty;
    State Open;

    // Events
    Event play;
    Event open_close;
    Event stop_event;
    Event cd_detected;
    Event pause;
    Event end_pause;
}
```

If you noticed, `Playing` is another state machine hidden among the CdPlayer's states declared above. State Machines are states too. When you have the ability to integrate them in this manner, you get a Hierarchical State Machine aka State Chart aka Harel State Chart. What does `Playing` look like?

```cpp
// Playing Hsm
struct Playing : public Hsm<PlayingHsm>
{

    // ... Some details have been left out...
    State* getStartState() override { return &Song1; }
    State* getStopState() override { return nullptr; }

    // States
    State Song1;
    State Song2;
    State Song3;

    // Events
    Event next_song;
    Event prev_song;

};

```

So there you have it. Let's see how the state transition table looks like for the CdPlayer.

##### State Transition Table

```cpp
// Goes in the header along with your State Machine class or .cpp file.
CdPlayerHsm::CdPlayerHsm()
      : Hsm<CdPlayerHsm>("CD Player Hsm")
      , Stopped("Player Stopped")
      , Paused("Player Paused")
      , Empty("Player Empty")
      , Open("Player Open")
    {
        // Tell Playing who's the parent. <---------------- Note
        Playing.setParent(this);

        // State Transition Table
        add(Stopped, play, <b>Playing</b>);
        add(Stopped, open_close, Open);
        add(Stopped, stop_event, Stopped);
        //-------------------------------------------------
        add(Open, open_close, Empty);
        //-------------------------------------------------
        add(Empty, open_close, Open);
        add(Empty, cd_detected, Stopped);
        add(Empty, cd_detected, <b>Playing</b>);
        //-------------------------------------------------
        add(<b>Playing</b>, stop_event, Stopped);
        add(<b>Playing</b>, pause, Paused);
        add(<b>Playing</b>, open_close, Open);
        //-------------------------------------------------
        add(Paused, end_pause, <b>Playing</b>);
        add(Paused, stop_event, Stopped);
        add(Paused, open_close, Open);
    }

```

`Playing` is just treated just as any other (atomic) state. A transition like `add(Stopped, play, <b>Playing</b>);`, is read as "When the machine is `Stopped` and gets a `play` event, it transitions to the `Playing` state". Is that all there is to the state transition table?

##### Actions and Guards
Looking at `Playing`'s state transition table,

```cpp
PlayingHsm()
  : Hsm<PlayingHsm>("Playing Hsm")
  , Song1()
  , Song2("Playing Hsm -> Song2")
  , Song3("Playing Hsm -> Song3")
{

    // clang-format off
    add(Song1, next_song, Song2, <b>&PlayingHsm::PlaySong, &PlayingHsm::PlaySongGuard</b>);
    // clang-format on
    add(Song2, next_song, Song3);
    add(Song3, prev_song, Song2);
    add(Song2, prev_song, Song1);
}

```

we notice `PlaySong` and `PlaySongGuard`. Guards of course prevent a transition from taking place. To make such decisions, they must have access to some additional information pertaining to the system.

```cpp
bool PlaySongGuard()
{
    // purely made up
    if (currentSong_.noiseLevel() > 50) {
        return false;
    }
    return true;
}
```

Actions perform an action *after* exiting the current state and *before* entering the next.

The point is they should know *something* about the system or current state. As seen for SwitchFsm, we can do:

```cpp
#include <tsm.h>
int main() {
    SingleThreadedHsm<CdPlayer> sm;
    sm.startSM();
    sm.sendEvent(play);
    sm.step(); // take the 'play' event from the event queue and transition to the next state
    // ...
    sm.stopSM();
}
```

or

```cpp
AsynchronousHsm<CdPlayer> sm;
sm.sendEvent(play);
```

Note that the call to `sm.step()` is not required for the `AsynchronousHsm`. Events are processed in a separate thread.

##### Start and Stop States
Also specify the start and stop states within `CdPlayer`

```cpp
State* getStartState() override { return &Song1; }
State* getStopState() override { return nullptr; }
```


#### Timer Driven State Machine
A whole class of problems can be solved in a much simpler manner with state machines that are driven by timers. The problem is to model traffic lights at a 2-way crossing. The states are G1(30s), Y1(5s), G2(60s), Y2(5s). The signal stays on for the amount of time indicated in the brackets adjacent to the signal before moving on to the next. The added complication is that G2 has a walk signal. If the walk signal is pressed, G2 stays on for only 30s instead of 60s before transitioning to Y2. The trick is to realize that there is only one event for this state machine: The expiry of a timer at say, 1s granularity. Such problems can be modeled by using timer driven state machines. Applications include game engines where a refresh of the game state happens every so many milliseconds, robotics, embedded software and of course traffic lights :).

```cpp
struct TrafficLightHsm : public Hsm<TrafficLightHsm>
{
    // ... with a few details removed...
    State* getStartState() override { return &G1; }
    State* getStopState() override { return nullptr; }

    // States
    LightState G1;
    LightState Y1;
    LightState G2;
    LightState Y2;

    // Events
    Event timer_event;

    // Walk button
    bool walkPressed;
    uint64_t ticks_;
};
```

To make it useful,

```cpp
#include <tsm.h>
using AsyncTrafficLightTimedHsm =
  TimedExecutionPolicy<AsynchronousHsm<TrafficLightHsm>,
                       tsm::ThreadSleepTimer>;
int main() {
    using namespace std::chrono_literals;
    // Painfully named as such to showcase capabilities.
    AsyncTrafficLightTimedHsm t(1s);
    t.startSM();
    // ...
}
```

`t.start()` will start a timer with a period of 1s. At the expiration of this timer, a `timer_event` will be placed in the event queue.

### Policy Based Design

We could just as well have created a timer driven traffic light in this manner:

```cpp
#include <tsm.h>
using TrafficLightTimedHsm =
  TimedExecutionPolicy<<b>SingleThreadedHsm</b><TrafficLightHsm>,
                       tsm::ThreadSleepTimer>;
int main() {
    using namespace std::chrono_literals;
    // Painfully named as such to showcase capabilities.
    TrafficLightTimedHsm t(1s);
    t.startSM();
    // ...
}
```

The only difference is that we are creating a TrafficLightTimedHsm where we have to drive the state machine with `t.step()` for event processing. That is flexible... and powerful. We can also write policies to
    * Persist state changes to log file
    * Calculate statistics on transitions
    * Write 'watchers' for particular conditions
    * Notify observers on events
Like it? Try it.

### Build

#### CMake only
If you don't want to install nix, `sudo apt install cmake ninja-build graphviz doxygen` or `brew install cmake ninja graphviz doxygen`. Read the `CMakeLists.txt` to get a good feel for it. The `cmake/superbuild` folder contains the cmake files that download and install the external dependencies. With Nix, none of these cmake files are needed. The dependencies and environment will be set up when you invoke `nix-shell` or `nix-build`.

```console
git clone https://github.com/tinverse/tsm.git
cd tsm; mkdir build; cd build
# cmake .. && make #if you want to use make
cmake -GNinja -DCMAKE_INSTALL_PREFIX=${HOME}/usr .. && ninja install
# run tests
./bin/tsm_test
```

### Integrating with your CMake project
How do I use it from my project? Look at the example project [CMakeLists.txt](https://github.com/tinverse/tsm/blob/master/examples/hello_tsm/CMakeLists.txt). Use this as a template for your project's CMakeLists.txt. The `tsm_DIR` variable should be set to point to the location of tsmConfig.cmake (for the case above, ${HOME}/usr/lib/cmake/tsm).

#### Nix - Recommended
Install nix by running `curl https://nixos.org/nix/install | sh`. The `default.nix` file is responsible for setting up your environment and installing all required dependencies.

```console
nix-shell --run ./build.sh
```

And you are done! If you open `build.sh` you'll notice that it uses ninja instead of make. `build.sh` creates a build folder, invokes cmake to generate the build scripts and then calls ninja or make via the `cmake --build` command to create the build outputs. You can find the test executable `tsm_test` under `build/test`, documentation under `build/docs/html` and coverage data under `test/tsm_test-coverage`. For your normal workflow,

```console
nix-shell
cd build
ninja tsm_test
```
You can also run `nix-build` from the command-line. This will build all targets and deploy them to `/nix/store`. A sym-link named `result` will be created in the cloned folder (the folder that contains default.nix) to the `/nix/store`. So to invoke `tsm_test`, run `./result/test/tsm_test`.

### Test Coverage
If making changes to tsm source, you can generate coverage reports as well. The cmake option `-DBUILD_COVERAGE=ON` turns it on. Feel free to steal this mechanism for your own projects. `ninja coverage` will invoke lcov and the report will be available under `test/tsm_test-coverage/index.html` in the build folder.

### Documentation
To generate doxygen docs, use the cmake option `-DBUILD_DOCUMENTATION=ON`. This can be invoked as needed - `ninja tsm_doc` or just plain `ninja` from the build folder.


### External Dependencies
None, if you just want to use the library. Just `#include <tsm>` and start using it! Catch2 unit test framework if you wish to run the unit tests. Set `BUILD_TESTING=OFF` in the CMakeCache file to disable building unit tests.

### Contribute
Please feel free to write up issues and submit pull requests. There is a .clang-format file that comes with the source. Coding conventions can be inferred by looking at other source files.

### TODOs
    * UML front end to define State Machine.
    * Implement concurrent execution policy for OrthogonalHsmExecutor.
