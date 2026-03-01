#include "catch2/catch_test_macros.hpp"
#include "lob/matching_engine.hpp"
#include <random>
#include <iostream>

using namespace lob;

TEST_CASE("MatchingEngine performance metrics", "[perf]") {
    const size_t POOL_SIZE = 1024;
    MatchingEngine engine(POOL_SIZE);

    // Random generator for prices and sides
    std::mt19937 rng(12345);
    std::uniform_int_distribution<int> side_dist(0, 1);  // 0=Buy, 1=Sell
    std::uniform_int_distribution<int> price_dist(90, 110);
    std::uniform_int_distribution<int> qty_dist(1, 20);

    const size_t NUM_ORDERS = 1000;

    for (size_t i = 1; i <= NUM_ORDERS; ++i) {
        auto* order = engine.book().pool().allocate();
        order->id        = i;
        order->side      = side_dist(rng) == 0 ? Side::Buy : Side::Sell;
        order->price     = price_dist(rng);
        order->qty       = qty_dist(rng);
        order->remaining = order->qty;
        order->ts        = i;

        std::cout << i << std::endl;

        // Measure latency of match
        auto start = std::chrono::high_resolution_clock::now();
        auto trades = engine.match_limit_order(order);
        auto end   = std::chrono::high_resolution_clock::now();

        double us = std::chrono::duration<double, std::micro>(end - start).count();
        engine.perf.record(us);
    }

    // Print stats
    std::cout << "Executed " << NUM_ORDERS << " orders\n";
    std::cout << "p50 latency: " << engine.perf.p50() << " us\n";
    std::cout << "p99 latency: " << engine.perf.p99() << " us\n";

    // Print pool allocation info
    std::cout << "Pool size: " << POOL_SIZE << "\n";
    std::cout << "Pool allocations: " << engine.book().pool().active() << "\n";
}