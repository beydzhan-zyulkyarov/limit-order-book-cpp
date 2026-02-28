#pragma once

#include <map>
#include <unordered_map>
#include "types.hpp"
#include "side.hpp"
#include "order.hpp"
#include "price_level.hpp"
#include "memory_pool.hpp"

namespace lob {

class OrderBook {
public:
    explicit OrderBook(std::size_t pool_size);

    Order* add_limit_order_no_match(
        OrderId id,
        Side side,
        Price price,
        Quantity qty,
        Timestamp ts
    );

    bool cancel_order(OrderId id);

    const PriceLevel* best_bid() const;
    const PriceLevel* best_ask() const;

    std::size_t size() const noexcept;

    auto& bids() noexcept { return bids_; }
    auto& asks() noexcept { return asks_; }

    const auto& bids() const noexcept { return bids_; }
    const auto& asks() const noexcept { return asks_; }

    OrderPool& pool() noexcept { return pool_; }
    const OrderPool& pool() const noexcept { return pool_; }

    void remove_from_level(Order* order);
    void insert_into_level(Order* order);

    auto& order_index() noexcept { return order_index_; }
    const auto& order_index() const noexcept { return order_index_; }

private:
    using BidLevels = std::map<Price, PriceLevel, std::greater<>>;
    using AskLevels = std::map<Price, PriceLevel, std::less<>>;

    BidLevels bids_;
    AskLevels asks_;

    std::unordered_map<OrderId, Order*> order_index_;

    OrderPool pool_;
};

} // namespace lob