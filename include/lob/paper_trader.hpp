#pragma once
#include "matching_engine.hpp"
#include <vector>
#include <string>
#include <functional>

namespace lob {

enum class EventType {
    LIMIT,   // New limit order
    CANCEL,  // Cancel existing order
    MODIFY   // Modify existing order (price/quantity)
};

struct HistoricalEvent {
    EventId id;             // unique identifier for event
    EventType type;         // LIMIT, CANCEL, MODIFY
    OrderId order_id;
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

using StrategyCallback = std::function<void(const TradeEvent&)>;

class PaperTradingEngine {
public:
    // --- Option 1: Construct internally ---
    explicit PaperTradingEngine(std::size_t pool_size)
        : owned_engine_(std::make_unique<MatchingEngine>(pool_size)),
          engine_(*owned_engine_) {}

    // --- Option 2: Use an external engine ---
    explicit PaperTradingEngine(MatchingEngine& external_engine)
        : engine_(external_engine) {}

    // Feed a sequence of historical events
    void feed_events(const std::vector<HistoricalEvent>& events);

    // Get executed trades
    const std::vector<TradeEvent>& trades() const noexcept { return trades_; }

    // Get analytics snapshots
    const std::vector<AnalyticsSnapshot>& analytics() const noexcept { return analytics_; }

    void set_strategy_callback(StrategyCallback cb) {
        callback_ = std::move(cb);
    }

    void replay_event(const HistoricalEvent& evt) {
        switch(evt.type) {
            case EventType::LIMIT: {
                Order* o = engine_.book().pool().allocate();
                o->id = evt.id;
                o->side = evt.side;
                o->price = evt.price;
                o->qty = evt.qty;
                o->remaining = evt.qty;
                o->ts = evt.ts;

                auto trades = engine_.match_limit_order(o);
                for (auto& t : trades) {
                    if (callback_) callback_(t);  // Notify strategy
                }
                break;
            }
            case EventType::CANCEL:
                engine_.book().cancel_order(evt.id);
                break;
            case EventType::MODIFY:
                engine_.book().modify_order(evt.id, evt.price, evt.qty);
                break;
        }
    }

private:
    // avoids default constructor requirement.
    std::unique_ptr<MatchingEngine> owned_engine_; // only used if constructed internally
    MatchingEngine& engine_;            // always the engine we operate on

    StrategyCallback callback_;

    std::vector<TradeEvent> trades_;
    std::vector<AnalyticsSnapshot> analytics_;

    // Capture analytics at each event
    void capture_snapshot(Timestamp ts);
};

} // namespace lob