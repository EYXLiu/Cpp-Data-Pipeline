#include <vector>
#include <tuple>
#include <algorithm>
#include <iostream>
#include "metrics_thread.hpp"
#include "metrics_queue.hpp"

void metricsThread(MetricsQueue& q) {
    std::vector<uint64_t> parse_samples;
    std::vector<uint64_t> dispatch_samples;
    std::vector<uint64_t> process_samples;
    std::vector<uint64_t> total_samples;

    // so we don't run into vector extend time latency
    parse_samples.reserve(1100);
    dispatch_samples.reserve(1100);
    process_samples.reserve(1100);
    total_samples.reserve(1100);

    while (true) {
        LatencyEvent e;

        while (q.pop(e)) {
            parse_samples.push_back(e.parse_ns);
            dispatch_samples.push_back(e.dispatch_ns);
            process_samples.push_back(e.process_ns);
            total_samples.push_back(e.total_ns);
            std::cout << "Total samples size: " << total_samples.size() << "\r" << std::flush;
        }
        if (total_samples.size() >= 1000) {
            auto report = [](std::vector<uint64_t>& v) {
                auto idx = [&](double p) {
                    return std::min(v.size() - 1, static_cast<size_t>((v.size() - 1) * p));
                };
                
                auto percentile = [&](double p) {
                    size_t i = idx(p);
                    std::nth_element(v.begin(), v.begin() + i, v.end());
                    return v[i];
                };

                return std::make_tuple(
                    percentile(0.50),
                    percentile(0.90),
                    percentile(0.99)
                );
            };

            auto [p50_parse, p90_parse, p99_parse] = report(parse_samples);
            auto [p50_dispatch, p90_dispatch, p99_dispatch] = report(dispatch_samples);
            auto [p50_proc, p90_proc, p99_proc] = report(process_samples);
            auto [p50_total, p90_total, p99_total] = report(total_samples);

            std::cout << "PARSE    p50=" << p50_parse / 1000.0 << "us p90=" << p90_parse / 1000.0 << "us p99=" << p99_parse / 1000.0 << "us\n";
            std::cout << "DISPATCH p50=" << p50_dispatch  / 1000.0 << "us p90=" << p90_dispatch  / 1000.0 << "us p99=" << p99_dispatch  / 1000.0 << "us\n";
            std::cout << "PROCESS  p50=" << p50_proc  / 1000.0 << "us p90=" << p90_proc  / 1000.0 << "us p99=" << p99_proc  / 1000.0 << "us\n";
            std::cout << "TOTAL    p50=" << p50_total / 1000.0 << "us p90=" << p90_total  / 1000.0 << "us p99=" << p99_total / 1000.0 << "us\n";
            std::cout << "-----------------------------\n";

            parse_samples.clear();
            process_samples.clear();
            total_samples.clear();
        }
    }
}