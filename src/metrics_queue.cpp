#include "metrics_queue.hpp"

void MetricsQueue::push(const LatencyEvent& e) {
    std::lock_guard<std::mutex> lock(m_);
    q_.push(e);
}

bool MetricsQueue::pop(LatencyEvent& out) {
    std::lock_guard<std::mutex> lock(m_);
    if (q_.empty()) return false;
    out = q_.front();
    q_.pop();
    return true;
}