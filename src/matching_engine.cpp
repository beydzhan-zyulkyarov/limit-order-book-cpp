#include <iostream>
#include "lob/matching_engine.hpp"

namespace lob {

MatchingEngine::MatchingEngine(std::size_t pool_size)
    : book_(pool_size)
{}

std::vector<TradeEvent> MatchingEngine::match_limit_order(Order* incoming)
{
    std::vector<TradeEvent> events;

    // Determine opposite book to match
    auto& opposite_book = (incoming->side == Side::Buy) ? book_.asks() : book_.bids();

    auto it = opposite_book.begin();

    while (it != opposite_book.end() && incoming->remaining > 0) {
        PriceLevel& level = it->second;

        // Check if price crosses
        bool cross = (incoming->side == Side::Buy) ? (incoming->price >= level.price)
                                                   : (incoming->price <= level.price);
        if (!cross) break;

        // Store next iterator BEFORE potentially erasing this price level
        auto next_it = std::next(it);

        // Match FIFO orders in this price level
        Order* resting = level.head;
        while (resting && incoming->remaining > 0) {
            Quantity executed_qty = std::min(incoming->remaining, resting->remaining);

            // Update quantities
            incoming->remaining -= executed_qty;
            resting->remaining -= executed_qty;
            level.total_volume -= executed_qty;

            // Record trade
            events.push_back({resting->id, incoming->id, resting->price, executed_qty, incoming->ts});

            // Remove fully filled resting order
            Order* next_resting = resting->next;
            if (resting->remaining == 0) {
                book_.remove_from_level(resting);  // removes from price level
                book_.order_index().erase(resting->id); // remove from index
                book_.pool().deallocate(resting);       // free memory
            }
            resting = next_resting;
        }

        // Move to the next price level
        it = next_it;
    }

    // If incoming still has remaining quantity, insert into book
    if (incoming->remaining > 0) {
        book_.insert_into_level(incoming);
        book_.order_index().emplace(incoming->id, incoming);
    } else {
        // Fully executed, deallocate
        book_.pool().deallocate(incoming);
    }

    return events;
}

} // namespace lob