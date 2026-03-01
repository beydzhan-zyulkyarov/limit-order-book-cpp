#include <catch2/catch_test_macros.hpp>
#include "lob/order_book.hpp"

using namespace lob;

TEST_CASE("Insert single bid order", "[orderbook]") {
    OrderBook book(1024);

    auto* o = book.add_limit_order_no_match(
        1, Side::Buy, 100, 10, 1000);

    REQUIRE(o != nullptr);
    REQUIRE(book.size() == 1);

    const PriceLevel* best = book.best_bid();
    REQUIRE(best != nullptr);
    REQUIRE(best->price == 100);
    REQUIRE(best->total_volume == 10);
    REQUIRE(best->head == o);
    REQUIRE(best->tail == o);
}

TEST_CASE("FIFO ordering at same price", "[orderbook]") {
    OrderBook book(1024);

    auto* o1 = book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);
    auto* o2 = book.add_limit_order_no_match(2, Side::Buy, 100, 20, 2);
    auto* o3 = book.add_limit_order_no_match(3, Side::Buy, 100, 30, 3);

    const PriceLevel* best = book.best_bid();
    REQUIRE(best->head == o1);
    REQUIRE(best->tail == o3);

    REQUIRE(o1->next == o2);
    REQUIRE(o2->next == o3);
    REQUIRE(o3->next == nullptr);

    REQUIRE(best->total_volume == 60);
}

TEST_CASE("Cancel middle order preserves links", "[orderbook]") {
    OrderBook book(1024);

    auto* o1 = book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);
    auto* o2 = book.add_limit_order_no_match(2, Side::Buy, 100, 20, 2);
    (void)o2;
    auto* o3 = book.add_limit_order_no_match(3, Side::Buy, 100, 30, 3);

    REQUIRE(book.cancel_order(2) == true);
    REQUIRE(book.size() == 2);

    const PriceLevel* best = book.best_bid();

    REQUIRE(best->head == o1);
    REQUIRE(best->tail == o3);

    REQUIRE(o1->next == o3);
    REQUIRE(o3->prev == o1);

    REQUIRE(best->total_volume == 40);
}

TEST_CASE("Cancel head order", "[orderbook]") {
    OrderBook book(1024);

    auto* o1 = book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);
    (void)o1;
    auto* o2 = book.add_limit_order_no_match(2, Side::Buy, 100, 20, 2);

    REQUIRE(book.cancel_order(1) == true);

    const PriceLevel* best = book.best_bid();
    REQUIRE(best->head == o2);
    REQUIRE(best->tail == o2);
    REQUIRE(best->total_volume == 20);
}

TEST_CASE("Cancel tail order", "[orderbook]") {
    OrderBook book(1024);

    auto* o1 = book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);
    auto* o2 = book.add_limit_order_no_match(2, Side::Buy, 100, 20, 2);
    (void)o2;

    REQUIRE(book.cancel_order(2) == true);

    const PriceLevel* best = book.best_bid();
    REQUIRE(best->head == o1);
    REQUIRE(best->tail == o1);
    REQUIRE(best->total_volume == 10);
}

TEST_CASE("Remove last order deletes price level", "[orderbook]") {
    OrderBook book(1024);

    book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);

    REQUIRE(book.cancel_order(1) == true);
    REQUIRE(book.size() == 0);

    REQUIRE(book.best_bid() == nullptr);
}

TEST_CASE("Bid and ask are independent", "[orderbook]") {
    OrderBook book(1024);

    book.add_limit_order_no_match(1, Side::Buy, 100, 10, 1);
    book.add_limit_order_no_match(2, Side::Sell, 105, 15, 2);

    const PriceLevel* bid = book.best_bid();
    const PriceLevel* ask = book.best_ask();

    REQUIRE(bid != nullptr);
    REQUIRE(ask != nullptr);

    REQUIRE(bid->price == 100);
    REQUIRE(ask->price == 105);
}