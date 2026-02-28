#include "lob/matching_engine.hpp"

namespace lob {

MatchingEngine::MatchingEngine(std::size_t pool_size)
    : book_(pool_size)
{}

std::vector<TradeEvent> MatchingEngine::match_limit_order(Order* incoming)
{
    std::vector<TradeEvent> events;

    // 1. Determine book side to match
    std::map<Price, PriceLevel>* opposite_book;

    if (incoming->side == Side::Buy) {
    // Match against asks
    auto& book_side = book_.asks();
    for (auto it = book_side.begin(); it != book_side.end(); /*...*/) {
        PriceLevel& level = it->second;
        // matching logic
    }
} else {
    // Match against bids
    auto& book_side = book_.bids();
    for (auto it = book_side.begin(); it != book_side.end(); /*...*/) {
        PriceLevel& level = it->second;
        // matching logic
    }
}

    auto it = opposite_book->begin();

    while (it != opposite_book->end() && incoming->remaining > 0) {
        PriceLevel& level = it->second;

        // Check price crossing
        bool cross = (incoming->side == Side::Buy) ? (incoming->price >= level.price)
                                                   : (incoming->price <= level.price);
        if (!cross) break;

        // Match FIFO orders at this price
        Order* resting = level.head;
        while (resting && incoming->remaining > 0) {
            Quantity executed_qty = std::min(incoming->remaining, resting->remaining);

            // Update orders
            incoming->remaining -= executed_qty;
            resting->remaining -= executed_qty;
            level.total_volume -= executed_qty;

            // Log trade
            events.push_back({resting->id, incoming->id, resting->price, executed_qty, incoming->ts});

            // Remove fully filled resting order
            Order* next_resting = resting->next;
            if (resting->remaining == 0) {
                book_.remove_from_level(resting);
                book_.pool().deallocate(resting);
            }
            resting = next_resting;
        }

        // Move to next price level
        if (!level.head) it = opposite_book->erase(it);
        else ++it;
    }

    // If incoming still has remaining, add to book
    if (incoming->remaining > 0) {
        book_.insert_into_level(incoming);
        book_.order_index().emplace(incoming->id, incoming);
    } else {
        book_.pool().deallocate(incoming);
    }

    return events;
}

} // namespace lob