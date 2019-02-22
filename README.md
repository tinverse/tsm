[![Build Status](https://travis-ci.org/tinverse/tsm.svg?branch=master)](https://travis-ci.org/tinverse/tsm)
[![codecov.io](http://codecov.io/github/tinverse/tsm/coverage.svg?branch=master)](http://codecov.io/github/tinverse/tsm?branch=master)

The [C++ API documentation](https://tinverse.github.io/tsm/index.html).

### What is tsm?
tsm is a flexible state machine framework with support for Hierarchical and Orthogonal State Machines.

### Features:
    * Hierarchical State machine.
    * Thread safe event queue - Parent and Separate thread execution context.
    * Ease of installation/distribution - Header only, CMake and Nix support.
    * Policy Based - Ability to customize behavior by defining execution policies.

### Usage
Assume you have a `CdPlayer` state machine. You can create an instance and send events to it.

```
#include <tsm.h>

SimpleStateMachine<CdPlayer> sm;
sm.send_event(play);
sm.step();
```

or

```cpp
AsyncStateMachine<CdPlayer> sm;
sm.send_event(play);
```

Note that the call to `sm.step()` is not required for the `AsyncStateMachine`. Events are processed in a separate thread.

#### StateMachine Class Definition
Your CdPlayer state machine will have States, Events, Actions, Guards and a Transition Table.  It could also  have sub HSMs or orthogonal state machines as states.

##### States, Events, Actions and Guards
```cpp
struct CdPlayer : public StateMachineDef<CdPlayer>
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
    StateMachine<PlayingHSMDef> Playing;
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

Actions and Guards are member functions with the above signatures. States and Events are data members.

##### State Transition Table
The state transition table is specified in `CdPlayer`'s constructor.

```csharp
CdPlayer(IHsmDef* parent = nullptr)
      : StateMachineDef<CdPlayer>("CD Player HSM", parent)
      , Stopped("Player Stopped")
      , Playing(this)
      , Paused("Player Paused")
      , Empty("Player Empty")
      , Open("Player Open")
    {
        // TransitionTable for CdPlayer HSM
        add(Stopped, play, Playing);
        add(Stopped, open_close, Open);
        add(Stopped, stop_event, Stopped);
        //-------------------------------------------------
        add(Open, open_close, Empty);
        //-------------------------------------------------
        add(Empty, open_close, Open);
        add(Empty, cd_detected, Stopped);
        add(Empty, cd_detected, Playing);
        //-------------------------------------------------
        add(Playing, stop_event, Stopped);
        add(Playing, pause, Paused);
        add(Playing, open_close, Open);
        //-------------------------------------------------
        add(Paused, end_pause, Playing);
        add(Paused, stop_event, Stopped);
        add(Paused, open_close, Open);
    }
```

##### Start and Stop States
Also specify the start and stop states within `CdPlayer`

```cpp
State* getStartState() override { return &Song1; }
State* getStopState() override { return nullptr; }
```

Please browse the [C++ API documentation](https://tinverse.github.io/tsm/index.html) for more details.

### Build

#### Nix - Highly recommended
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

Other build targets are `coverage` `tsm_doc`. These can be invoked as needed - `ninja coverage`, `ninja tsm_doc` or just plain `ninja` from the build folder.

You can also run `nix-build` from the command-line. This will build all targets and deploy them to `/nix/store`. A sym-link named `result` will be created in the cloned folder (the folder that contains default.nix) to the `/nix/store`. So to invoke tsm_test, run `./result/test/tsm_test`.

#### CMake only
If you don't want to install nix, `sudo apt install cmake ninja-build graphviz doxygen` or `brew install cmake ninja graphviz doxygen`. Read the `CMakeLists.txt` to get a good feel for it. The `cmake/superbuild` folder contains the cmake files that download and install the external dependencies. With Nix, none of these cmake files are needed. The dependencies and environment will be set up when you invoke `nix-shell` or `nix-build`.

```console
git clone https://github.com/tinverse/tsm.git
cd tsm; mkdir build; cd build
# cmake .. && make #if you want to use make
cmake -GNinja .. && ninja
./test/tsm_test
```

### External Dependencies:
    Gflags, Glog, Gtest

### TODOs
    * UML front end to define State Machine.
    * Expand OrthogonalStateMachine support to arbitrary number of HSMs.
    * Implement concurrent execution policy for OrthogonalStateMachine.
