#ifndef LATENCY_TRACK_HPP
#define LATENCY_TRACK_HPP

#include <stdint.h>

struct LatencyEvent {
    uint64_t parse_ns;
    uint64_t dispatch_ns;
    uint64_t process_ns;
    uint64_t total_ns;
};

#endif
