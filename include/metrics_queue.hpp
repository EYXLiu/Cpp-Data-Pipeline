#ifndef METRICS_QUEUE_HPP
#define METRICS_QUEUE_HPP
#include <queue>
#include <mutex>
#include "latency_event.hpp"

class MetricsQueue {
public:
    void push(const LatencyEvent& e);
    bool pop(LatencyEvent& out);
private:
// technically slow but accetable for first mvp (minimum viable product)
    std::queue<LatencyEvent> q_;
    std::mutex m_;
};

#endif
