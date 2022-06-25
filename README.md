## Build and run on Linux with mockup files for testing, no CC1200 support:
```
mkdir build
cd build
cmake -DBUILD_LOCAL_ONLY=ON .
cmake --build ..
```

## Build and run on Beaglebone with full support:
```
mkdir build
cd build
cmake .
cmake --build ..
```
### Build without testing flags
It's now possible to build everything without unit tests.
Add this flag to cmake when configuring:
```
cmake -DBUILD_TESTS=OFF .
```