#include <catch2/catch_test_macros.hpp>
#include "lob/matching_engine.hpp"

using namespace lob;

TEST_CASE("Single cross") {
    MatchingEngine engine(1024);

    auto* resting = engine.book().add_limit_order_no_match(1, Side::Sell, 100, 10, 1);
    auto* incoming = engine.book().pool().allocate();
    incoming->id = 2;
    incoming->side = Side::Buy;
    incoming->price = 100;
    incoming->qty = 10;
    incoming->remaining = 10;
    incoming->ts = 2;

    auto trades = engine.match_limit_order(incoming);

    REQUIRE(trades.size() == 1);
    REQUIRE(trades[0].quantity == 10);
    REQUIRE(engine.book().size() == 0);
}