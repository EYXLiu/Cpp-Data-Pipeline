#include <reactor.hpp>
#include <buffer_pool.hpp>
#include <acceptor.hpp>
#include <kraken_ingestor.hpp>
#include <thread>
#include <iostream>
#include <latency_event.hpp>
#include <metrics_thread.hpp>

MetricsQueue metrics_q;

void handleBookUpdate(BookUpdate& u) {
    uint64_t start = KrakenDataIngestor::now_ns();
    
    LatencyEvent e;
    e.parse_ns = u.t_parsed - u.t_ingest;
    e.dispatch_ns = start - u.t_parsed;

    uint64_t end = KrakenDataIngestor::now_ns();

    e.process_ns = end - start;
    e.total_ns = end - u.t_ingest;

    metrics_q.push(e);
}

int main() {
    std::thread t(metricsThread, std::ref(metrics_q));
    t.detach();

    KrakenDataIngestor ingestor(handleBookUpdate);
    ingestor.start();
}
