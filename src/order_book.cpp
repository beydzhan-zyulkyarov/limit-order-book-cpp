#include "lob/order_book.hpp"
#include <iostream>

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
        return false;  // Order not found
    }

    Order* order = it->second;

    // Remove from the appropriate price level
    remove_from_level(order);

    // Deallocate the order from the pool and remove it from the index
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
    auto& book_side = (order->side == Side::Buy) ? bids_ : asks_;
    auto it = book_side.find(order->price);

    if (it == book_side.end()) {
        // Create new price level
        PriceLevel level{};
        level.price        = order->price;
        level.total_volume = order->remaining;
        level.head         = order;
        level.tail         = order;

        book_side.emplace(order->price, level);
    } else {
        PriceLevel& level = it->second;

        if (!level.tail) {
            // Level exists but empty — treat as new
            level.head = order;
            level.tail = order;
            level.total_volume = order->remaining;
            order->prev = nullptr;
            order->next = nullptr;
        } else {
            // Append to existing tail
            order->prev = level.tail;
            level.tail->next = order;
            level.tail = order;
            level.total_volume += order->remaining;
            order->next = nullptr;
        }
    }
}

void OrderBook::remove_from_level(Order* order)
{
    if (!order) return;

    PriceLevel* level = nullptr;

    if (order->side == Side::Buy) {
        auto it = bids_.find(order->price);
        if (it == bids_.end()) return;
        level = &it->second;
    } else {
        auto it = asks_.find(order->price);
        if (it == asks_.end()) return;
        level = &it->second;
    }

    // Remove from linked list
    if (order->prev) order->prev->next = order->next;
    if (order->next) order->next->prev = order->prev;

    if (level->head == order) level->head = order->next;
    if (level->tail == order) level->tail = order->prev;

    level->total_volume -= order->remaining;

    // If the level is now empty, erase it from the map
    if (level->head == nullptr) {
        if (order->side == Side::Buy) {
            bids_.erase(order->price);
        } else {
            asks_.erase(order->price);
        }
    }

    // At this point, the level is safely removed if empty
}

} // namespace lob