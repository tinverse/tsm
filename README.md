### Goals:
    Production ready Hierarchical State machine. The event queue is thread safe.
    C++14 - no manual memory management, C++ std libraries only
    Ease of installation/ distribution

### External Dependencies:
    GLog, Gtest

### Build:
Install cmake (sudo apt install cmake) or download on windows and add cmake to system path.

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

