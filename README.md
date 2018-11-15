[![Build Status](https://travis-ci.org/tinverse/tsm.svg?branch=master)](https://travis-ci.org/tinverse/tsm)
[![codecov.io](http://codecov.io/github/tinverse/tsm/coverage.svg?branch=master)](http://codecov.io/github/tinverse/tsm?branch=master)

The [C++ API documentation](https://tinverse.github.io/tsm/index.html).

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

### Build

#### Nix - Highly recommended
Install nix by running `curl https://nixos.org/nix/install | sh`. The `default.nix` file is responsible for setting up your environment and installing all required dependencies.

```
nix-shell --run ./build.sh
```

And you are done! If you open `build.sh` you'll notice that it uses ninja instead of make. `build.sh` creates a build folder, invokes cmake to generate the build scripts and then calls ninja or make via the `cmake --build` command to create the build outputs. You can find the test executable `tsm_test` under `build/test`, documentation under `build/docs/html` and coverage data under `test/tsm_test-coverage`. For your normal workflow,

```
nix-shell
cd build
ninja tsm_test
```
Other build targets are `coverage` `tsm_doc`. These can be invoked as needed - `ninja coverage`, `ninja tsm_doc` or just plain `ninja` from the build folder.

You can also run `nix-build` from the command-line. This will build all targets and deploy them to `/nix/store`. A sym-link named `result` will be created in the cloned folder (the folder that contains default.nix) to the `/nix/store`. So to invoke tsm_test, run `./result/test/tsm_test`.

#### CMake only
If you don't want to install nix, `sudo apt install cmake ninja-build graphviz doxygen` or `brew install cmake ninja graphviz doxygen`. Read the `CMakeLists.txt` to get a good feel for it. The `cmake/superbuild` folder contains the cmake files that download and install the external dependencies. With Nix, none of these cmake files are needed. The dependencies and environment will be set up when you invoke `nix-shell` or `nix-build`.

```
git clone https://github.com/tinverse/tsm.git
cd tsm
mkdir build
cd build
cmake -GNinja -DBUILD_DEPENDENCIES=ON ..
ninja
cmake -GNinja -DBUILD_DEPENDENCIES=OFF ..
ninja
./test/tsm_test
```

Please browse the [C++ API documentation](https://tinverse.github.io/tsm/index.html).
