# Rate Adaptation MINSTREL algorithm implementation for a Texas Instrument CC1200 HF-Transceiver on a Beaglebone with Debian

## This project is still in progress. There will be an update as well as an more detailed installation manual.

## Build and run on Beaglebone with full support:
```
mkdir build
cd build
cmake .
cmake --build ..
```

## Settings for testing

### Build and run on Linux with mockup files for testing, no CC1200 support:
```
mkdir build
cd build
cmake -DBUILD_LOCAL_ONLY=ON ..
cmake --build ..
```

### Build without testing flags
It's possible to build everything without unit tests.
Add this flag to cmake when configuring:
```
cmake -DBUILD_TESTS=OFF .
```

### Minstrel update cycle
The minstrel algorithm is keeping track of three rate types
- Best throughput
- 2nd best throughtput
- Highest probability

The throughput (T) of a rate is calculated as:
$T = {succ\textunderscore prob\enspace \cdot \enspace bytes\textunderscore send \over duration}$
- succ_prob: success probability of the given rate. As a simple heuristic, we take packets_send/acks_received
- bytes_send: total bytes per packet
- duration: average time from sending till receiving an acknowledgment of the same packet

To smoothen the success probability over time (while giving more weight to recent events), we apply an exponential moving average:
$succ\textunderscore prob_t = {\alpha \enspace \cdot \enspace {send \over acks} \enspace + \enspace (1 - \alpha) \enspace \cdot \enspace succ\textunderscore prob_{t-1}}$

(Adapted from [Rate Adaptation for 802.11 Wireless Networks: Minstrel
](https://blog.cerowrt.org/papers/minstrel-sigcomm-final.pdf))
