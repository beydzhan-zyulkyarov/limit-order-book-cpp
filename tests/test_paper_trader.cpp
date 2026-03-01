#include "lob/paper_trader.hpp"
#include "catch2/catch_test_macros.hpp"
#include <iostream>

using namespace lob;

TEST_CASE("PaperTradingEngine handles LIMIT, CANCEL, MODIFY events correctly") {
    // 1️⃣ Create paper trading engine with pool size 1024
    PaperTradingEngine paper(1024);

    // 2️⃣ Define event sequence
    std::vector<HistoricalEvent> events = {
        // Add buy order
        {1, EventType::LIMIT, 1, Side::Buy, 100, 10, 1},
        // Add sell order
        {2, EventType::LIMIT, 2, Side::Sell, 101, 5, 2},
        // Modify buy order to higher price
        {3, EventType::MODIFY, 1, Side::Buy, 102, 12, 3},
        // Cancel sell order
        {4, EventType::CANCEL, 2, Side::Sell, 0, 0, 4},
        // Add sell order that crosses with buy
        {5, EventType::LIMIT, 5, Side::Sell, 101, 8, 5}
    };

    // 3️⃣ Feed events
    paper.feed_events(events);

    // 4️⃣ Check trades
    const auto& trades = paper.trades();
    REQUIRE(trades.size() > 0);

    // Expect at least one trade from buy@102 vs sell@101
    bool found_trade = false;
    for (const auto& t : trades) {
        if (t.price == 101 && t.quantity > 0) found_trade = true;
    }
    REQUIRE(found_trade);

    // 5️⃣ Check analytics snapshots
    const auto& snapshots = paper.analytics();
    REQUIRE(snapshots.size() == events.size());

    // Check mid-price and total volume are reasonable
    for (const auto& snap : snapshots) {
        REQUIRE(snap.mid_price >= 0);
        REQUIRE(snap.total_volume >= 0);
    }

    // 6️⃣ Optional: print trades and snapshots
    std::cout << "Trades:\n";
    for (const auto& t : trades) {
        std::cout << "Trade: resting=" << t.resting_order_id
                  << " incoming=" << t.incoming_order_id
                  << " price=" << t.price
                  << " qty=" << t.quantity
                  << " ts=" << t.ts << "\n";
    }

    std::cout << "\nAnalytics snapshots:\n";
    for (const auto& snap : snapshots) {
        std::cout << "TS=" << snap.ts
                  << " mid=" << snap.mid_price
                  << " total_vol=" << snap.total_volume << "\n";
    }
}