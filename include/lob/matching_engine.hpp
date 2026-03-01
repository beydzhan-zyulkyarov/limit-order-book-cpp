#pragma once
#include "order_book.hpp"
#include <vector>
#include <chrono>
#include <algorithm>

namespace lob {

struct TradeEvent {
    OrderId resting_order_id;
    OrderId incoming_order_id;
    Price price;
    Quantity quantity;
    Timestamp ts;
};

struct PerfStats {
    std::vector<double> match_latencies_us; // microseconds

    void record(double us) { match_latencies_us.push_back(us); }

    double percentile(double p) {
        if (match_latencies_us.empty()) 
            return 0.0;

        std::vector<double> tmp = match_latencies_us;
        std::sort(tmp.begin(), tmp.end());

        // safe conversion
        size_t n = tmp.size();
        size_t idx = static_cast<size_t>(p * static_cast<double>(n));
        
        if (idx >= n) 
            idx = n - 1;

        return tmp[idx];
    }

    double p50() { return percentile(0.5); }
    double p99() { return percentile(0.99); }
};

class MatchingEngine {
public:
    explicit MatchingEngine(std::size_t pool_size);

    // Main entry point
    std::vector<TradeEvent> match_limit_order(Order* incoming);

    // Access book for analytics
    OrderBook& book() noexcept { return book_; }
    const OrderBook& book() const noexcept { return book_; }

    // ----------------------
    // Performance metrics
    // ----------------------
    PerfStats perf;

private:
    OrderBook book_;
};

} // namespace lob