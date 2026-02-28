#include "include/lob/order_book.hpp"

namespace lob {

// ---------------- Constructor ----------------

OrderBook::OrderBook(std::size_t pool_size)
    : pool_(pool_size)
{
}

// ---------------- Public API ----------------

Order* OrderBook::add_limit_order_no_match(
    OrderId id,
    Side side,
    Price price,
    Quantity qty,
    Timestamp ts)
{
    // Allocate from pool
    Order* order = pool_.allocate();
    if (!order) {
        return nullptr; // or throw if preferred
    }

    // Initialize fields
    order->id        = id;
    order->price     = price;
    order->qty       = qty;
    order->remaining = qty;
    order->side      = side;
    order->ts        = ts;
    order->next      = nullptr;
    order->prev      = nullptr;

    // Insert into price level
    insert_into_level(order);

    // Add to id index
    order_index_.emplace(id, order);

    return order;
}

bool OrderBook::cancel_order(OrderId id)
{
    auto it = order_index_.find(id);
    if (it == order_index_.end()) {
        return false;
    }

    Order* order = it->second;

    remove_from_level(order);

    pool_.deallocate(order);
    order_index_.erase(it);

    return true;
}

const PriceLevel* OrderBook::best_bid() const
{
    if (bids_.empty())
        return nullptr;

    return &bids_.begin()->second;
}

const PriceLevel* OrderBook::best_ask() const
{
    if (asks_.empty())
        return nullptr;

    return &asks_.begin()->second;
}

std::size_t OrderBook::size() const noexcept
{
    return order_index_.size();
}

// ---------------- Internal Helpers ----------------

void OrderBook::insert_into_level(Order* order)
{
    if (order->side == Side::Buy) {
        auto it = bids_.find(order->price);

        if (it == bids_.end()) {
            PriceLevel level{};
            level.price        = order->price;
            level.total_volume = order->remaining;
            level.head         = order;
            level.tail         = order;

            bids_.emplace(order->price, level);
        } else {
            PriceLevel& level = it->second;

            order->prev = level.tail;
            level.tail->next = order;
            level.tail = order;

            level.total_volume += order->remaining;
        }
    } else {
        auto it = asks_.find(order->price);

        if (it == asks_.end()) {
            PriceLevel level{};
            level.price        = order->price;
            level.total_volume = order->remaining;
            level.head         = order;
            level.tail         = order;

            asks_.emplace(order->price, level);
        } else {
            PriceLevel& level = it->second;

            order->prev = level.tail;
            level.tail->next = order;
            level.tail = order;

            level.total_volume += order->remaining;
        }
    }
}

void OrderBook::remove_from_level(Order* order)
{
    if (order->side == Side::Buy) {
        auto it = bids_.find(order->price);
        if (it == bids_.end()) return;

        PriceLevel& level = it->second;

        if (order->prev)
            order->prev->next = order->next;
        else
            level.head = order->next;

        if (order->next)
            order->next->prev = order->prev;
        else
            level.tail = order->prev;

        level.total_volume -= order->remaining;

        if (!level.head)
            bids_.erase(it);
    }
    else {
        auto it = asks_.find(order->price);
        if (it == asks_.end()) return;

        PriceLevel& level = it->second;

        if (order->prev)
            order->prev->next = order->next;
        else
            level.head = order->next;

        if (order->next)
            order->next->prev = order->prev;
        else
            level.tail = order->prev;

        level.total_volume -= order->remaining;

        if (!level.head)
            asks_.erase(it);
    }

    order->next = nullptr;
    order->prev = nullptr;
}

} // namespace lob