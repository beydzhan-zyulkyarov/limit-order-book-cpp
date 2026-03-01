#pragma once

#include "types.hpp"
#include "order.hpp"

struct PriceLevel {
    Price price{};
    Quantity total_volume{0};
    
    Order* head{nullptr};
    Order* tail{nullptr};
};