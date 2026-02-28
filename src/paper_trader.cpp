#include "lob/paper_trader.hpp"
#include <algorithm>

namespace lob {

PaperTradingEngine::PaperTradingEngine(std::size_t pool_size)
    : engine_(pool_size)
{}

void PaperTradingEngine::feed_events(const std::vector<HistoricalEvent>& events)
{
    for (const auto& e : events) {
        // Allocate order
        auto* o = engine_.book().pool().allocate();
        o->id = e.id;
        o->side = e.side;
        o->price = e.price;
        o->qty = e.qty;
        o->remaining = e.qty;
        o->ts = e.ts;

        // Execute matching
        auto event_trades = engine_.match_limit_order(o);
        trades_.insert(trades_.end(), event_trades.begin(), event_trades.end());

        // Capture analytics
        capture_snapshot(e.ts);
    }
}

void PaperTradingEngine::capture_snapshot(Timestamp ts)
{
    // Simplest analytics: mid-price & total volume
    auto& book = engine_.book();
    Price best_bid = book.bids().empty() ? 0 : book.bids().begin()->first;
    Price best_ask = book.asks().empty() ? 0 : book.asks().begin()->first;

    Quantity total_volume = 0;
    for (const auto& [price, level] : book.bids()) total_volume += level.total_volume;
    for (const auto& [price, level] : book.asks()) total_volume += level.total_volume;

    AnalyticsSnapshot snap{ts, (best_bid + best_ask) / 2, total_volume};
    analytics_.push_back(snap);
}

} // namespace lob