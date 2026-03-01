#pragma once

#include <vector>
#include <map>
#include <chrono>
#include <algorithm>
#include <cstdint>
#include <mutex>
#include <iostream>
#include "types.hpp"

namespace lob {

// ------------------------
// Timestamp helper
// ------------------------
using Timestamp = uint64_t;

inline Timestamp current_timestamp() {
    return static_cast<Timestamp>(
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count()
    );
}

// ------------------------
// PerfStats: Track latencies & allocations
// ------------------------
struct PerfStats {
    std::mutex mtx;

    std::vector<double> latencies_us;   // microseconds per match
    size_t pool_alloc_count = 0;

    void record(double us) {
        std::lock_guard<std::mutex> lock(mtx);
        latencies_us.push_back(us);
    }

    void record_allocation() {
        std::lock_guard<std::mutex> lock(mtx);
        ++pool_alloc_count;
    }

    double percentile(double p) {
        if (latencies_us.empty()) 
            return 0.0;

        std::vector<double> tmp = latencies_us;
        std::sort(tmp.begin(), tmp.end());

        // safe conversion
        size_t n = tmp.size();
        size_t idx = static_cast<size_t>(p * static_cast<double>(n));
        
        if (idx >= n) 
            idx = n - 1;

        return tmp[idx];
    }

    double p50() { return percentile(0.5); }
    double p99() { return percentile(0.99); }

    void clear() {
        std::lock_guard<std::mutex> lock(mtx);
        latencies_us.clear();
        pool_alloc_count = 0;
    }
};

// ------------------------
// PriceLevel snapshot
// ------------------------
struct PriceLevelSnapshot {
    Price price;
    uint64_t total_volume;
    size_t order_count;
};

// ------------------------
// Snapshot: top N levels of book
// ------------------------
struct BookSnapshot {
    Timestamp ts;
    std::vector<PriceLevelSnapshot> top_bids;
    std::vector<PriceLevelSnapshot> top_asks;

    void print() const {
        std::cout << "Snapshot @ " << ts << " us\n";
        std::cout << "Top Bids:\n";
        for (auto it = top_bids.rbegin(); it != top_bids.rend(); ++it) {
            std::cout << "  " << it->price << " -> " << it->total_volume << " (" << it->order_count << " orders)\n";
        }
        std::cout << "Top Asks:\n";
        for (auto& lvl : top_asks) {
            std::cout << "  " << lvl.price << " -> " << lvl.total_volume << " (" << lvl.order_count << " orders)\n";
        }
    }
};

// ------------------------
// Capture snapshot from OrderBook
// ------------------------
template<typename OrderBook>
inline BookSnapshot capture_snapshot(const OrderBook& book, size_t top_n = 5) {
    BookSnapshot snap;
    snap.ts = current_timestamp();

    // Capture top bids (sorted descending)
    size_t count = 0;
    for (auto it = book.bids().rbegin(); it != book.bids().rend() && count < top_n; ++it, ++count) {
        const auto& lvl = it->second;
        size_t orders = 0;
        for (Order* o = lvl.head; o; o = o->next) ++orders;
        snap.top_bids.push_back({lvl.price, lvl.total_volume, orders});
    }

    // Capture top asks (sorted ascending)
    count = 0;
    for (auto it = book.asks().begin(); it != book.asks().end() && count < top_n; ++it, ++count) {
        const auto& lvl = it->second;
        size_t orders = 0;
        for (Order* o = lvl.head; o; o = o->next) ++orders;
        snap.top_asks.push_back({lvl.price, lvl.total_volume, orders});
    }

    return snap;
}

/* USAGE: Take snapshots periodically
    auto snap = lob::capture_snapshot(book_, 10);
    snap.print();
*/

} // namespace lob