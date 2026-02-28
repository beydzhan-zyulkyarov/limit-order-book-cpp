#pragma once
#include "matching_engine.hpp"
#include <vector>
#include <string>

namespace lob {

struct HistoricalEvent {
    OrderId id;
    Side side;
    Price price;
    Quantity qty;
    Timestamp ts;
    // Could add event type: Limit / Cancel / Modify
};

struct AnalyticsSnapshot {
    Timestamp ts;
    Price mid_price;
    Quantity total_volume;
    // Additional metrics: bid/ask spread, depth, etc.
};

class PaperTradingEngine {
public:
    explicit PaperTradingEngine(std::size_t pool_size);

    // Feed a sequence of historical events
    void feed_events(const std::vector<HistoricalEvent>& events);

    // Get executed trades
    const std::vector<TradeEvent>& trades() const noexcept { return trades_; }

    // Get analytics snapshots
    const std::vector<AnalyticsSnapshot>& analytics() const noexcept { return analytics_; }

private:
    MatchingEngine engine_;
    std::vector<TradeEvent> trades_;
    std::vector<AnalyticsSnapshot> analytics_;

    // Capture analytics at each event
    void capture_snapshot(Timestamp ts);
};

} // namespace lob