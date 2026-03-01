#pragma once
#include "order_book.hpp"
#include "perf_snapshots.hpp"
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