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
    uint64_t now = KrakenDataIngestor::now_ns();
    u.t_processed = now;
    
    LatencyEvent e;
    e.parse_ns = u.t_parsed - u.t_ingest;
    e.process_ns = u.t_processed - u.t_parsed;
    e.total_ns = u.t_processed - u.t_ingest;
    e.timestamp = KrakenDataIngestor::now_ns();

    metrics_q.push(e);
}

int main() {
    std::thread t(metricsThread, std::ref(metrics_q));
    t.detach();

    KrakenDataIngestor ingestor(handleBookUpdate);
    ingestor.start();
}
