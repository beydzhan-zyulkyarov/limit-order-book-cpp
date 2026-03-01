# Limit Order Book — C++ Implementation

High‑performance limit order book written in modern C++ (header‑only API), designed to track and match orders with price‑time priority — suitable for backtesting, simulations, and integration in low‑latency trading systems.

## Overview

A limit order book (LOB) is a core data structure in financial markets. It maintains bids and asks, matching incoming orders (market, limit, cancel, modify) with existing orders based on price priority and time priority. This implementation focuses on simplicity, correctness, and extensibility.

## Features

✔ Price‑time priority matching — matches best price first, FIFO within price level
✔ Supports market and limit orders
✔ Supports order modification and cancellation
✔ Minimal, header‑only API for easy embedding in other C++ projects
✔ Designed for clarity with options to optimize further
✔ Clean CMake build and test structure
✔ Includes example usage and simple performance tests

## Design & Architecture

The order book is typically structured as:

Price levels: organized by price (e.g., using sorted maps, trees, or flat arrays)

Orders per price level: stored in a linked list or deque to maintain time priority

Matching engine: walks the best bid/ask side to fill orders as they arrive

This implementation focuses on:

✔ O(log M) for new price levels where M is number of distinct prices.
✔ O(1) for order insertions at existing price levels.
✔ FIFO dispatch within price levels.
✔ Clean API for common operations:

LOB book;
book.add_limit(Order{...});
book.cancel(order_id);
book.modify(order_id, new_qty, new_price);
book.match(); // attempt match

(Example interface — adjust to your API)

## Installation & Build

### Requirements:

✔ C++17 or later
✔ CMake 3.16+
✔ A modern C++ compiler (GCC/Clang/MSVC)

### Build:

git clone https://github.com/beydzhan-zyulkyarov/limit-order-book-cpp.git
mkdir build && cd build
cmake ..
cmake --build . --parallel
🧪 Testing

Unit tests validate correctness of order placement, matching, modification, cancellation, and edge cases.

Run tests with:

cd build
ctest --output-on-failure

or (if using Google Test directly):

./orderbook_tests

## Benchmarking

Lightweight benchmarks are included under benchmarks/ to measure raw order book throughput and latency. These help you reason about trade rates in microseconds per order.

(Add specific commands if present, e.g., ./bench_orderbook)

Tip: For more realistic benchmarks, combine with synthetic or historical market ticks.

## Example Usage

Here is a simple snippet showing basic interactions:

#include "order_book.hpp"
#include "order_types.hpp"

int main() {
    OrderBook lob;

    // Add limit orders
    lob.add_limit({1, Side::Buy, 100.5, 10});
    lob.add_limit({2, Side::Sell, 101.0, 5});

    // Market order
    auto trades = lob.match_market({3, Side::Buy, 0.0, 20});

    // Cancel order
    lob.cancel(2);

    // Print top of book
    std::cout << "Best Bid: " << lob.best_bid() << "\n";
    std::cout << "Best Ask: " << lob.best_ask() << "\n";

    return 0;
}

(Adapt to your actual API)

## Goals & Use Cases

This project is aimed at:

- Backtesting trading strategies
- Simulating market behaviour
- Educational purposes — learn inner workings of matching engines
- Research and performance tuning

It’s not a full exchange or a production matching engine, but a solid building block toward one.

## Contributing

Contributions welcome — improve matching logic, add more order types, integrate simulators, optimize data structures and algorithms.

Please follow standard conventions:

✔ Add tests
✔ Document API changes
✔ Benchmark performance after significant modifications

## What’s Next?

For future enhancements:

⭐ Add stop / stop‑limit / iceberg order support
⭐ More realistic ordered event replay benchmarking
⭐ Multi‑threaded ingestion pipeline
⭐ Integration with real market data feeds

## License

MIT License — free to use and extend.
