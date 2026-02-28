#include <iostream>
#include "lob/paper_trader.hpp"

using namespace lob;

int main() {
    std::vector<HistoricalEvent> events = {
        {1, EventType::LIMIT, 1, Side::Buy, 100, 10, 1},      // New buy order
        {2, EventType::LIMIT, 2, Side::Sell, 101, 5, 2},      // New sell order
        {3, EventType::MODIFY, 1, Side::Buy, 102, 12, 3},     // Modify order 1
        {4, EventType::CANCEL, 2, Side::Sell, 0, 0, 4}       // Cancel order 2
    };

    PaperTradingEngine paper(1024);
    paper.feed_events(events);

    // Print trades
    for (auto& t : paper.trades()) {
        std::cout << t.incoming_order_id << "@" << t.price << " x " << t.quantity << "\n";
    }

    // Print analytics snapshots
    for (auto& snap : paper.analytics()) {
        std::cout << snap.ts << " mid=" << snap.mid_price
                  << " vol=" << snap.total_volume << "\n";
    }
}