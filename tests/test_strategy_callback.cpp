#include <catch2/catch_test_macros.hpp>
#include "lob/paper_trader.hpp"
#include "lob/side.hpp"

TEST_CASE("PaperTradingEngine strategy callback", "[strategy]") {
    lob::MatchingEngine engine(1024);
    lob::PaperTradingEngine paper(engine);

    bool callback_called = false;

    // Set callback to verify it is triggered
    paper.set_strategy_callback([&](const lob::TradeEvent& t){
        callback_called = true;
        REQUIRE(t.quantity == 10);
        REQUIRE(t.price == 100);
    });

    // Create a resting sell order
    auto* resting = engine.book().add_limit_order_no_match(1, Side::Sell, 100, 10, 1);
    REQUIRE(resting != nullptr);

    // Create an incoming buy order as a historical event
    lob::HistoricalEvent evt;
    evt.type = lob::EventType::LIMIT;
    evt.id = 2;
    evt.side = Side::Buy;
    evt.price = 100;
    evt.qty = 10;
    evt.ts = 2;

    // Replay the event; should trigger callback
    paper.replay_event(evt);

    REQUIRE(callback_called);   // Confirm callback executed
    REQUIRE(engine.book().size() == 0);  // Both orders matched
}