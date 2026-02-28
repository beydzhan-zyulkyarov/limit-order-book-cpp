#pragma once

#include <vector>
#include "order.hpp"

namespace lob {

class OrderPool {
public:
    explicit OrderPool(std::size_t capacity)
    {
        storage_.resize(capacity);

        // Build free list
        for (std::size_t i = 0; i < capacity; ++i) {
            free_list_.push_back(&storage_[i]);
        }
    }

    Order* allocate()
    {
        if (free_list_.empty())
            return nullptr; // or throw

        Order* o = free_list_.back();
        free_list_.pop_back();
        return o;
    }

    void deallocate(Order* o)
    {
        free_list_.push_back(o);
    }

private:
    std::vector<Order> storage_;
    std::vector<Order*> free_list_;
};

} // namespace lob