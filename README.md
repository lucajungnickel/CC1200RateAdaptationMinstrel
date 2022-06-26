## Rate Adaptation MINSTREL algorithm implementation for a Texas Instrument CC1200 HF-Transceiver on a Beaglebone with Debian

## Build and run on Beaglebone with full support:
```
mkdir build
cd build
cmake .
cmake --build ..
```

##Settings for testing

### Build and run on Linux with mockup files for testing, no CC1200 support:
```
mkdir build
cd build
cmake -DBUILD_LOCAL_ONLY=ON .
cmake --build ..
```

### Build without testing flags
It's possible to build everything without unit tests.
Add this flag to cmake when configuring:
```
cmake -DBUILD_TESTS=OFF .
```