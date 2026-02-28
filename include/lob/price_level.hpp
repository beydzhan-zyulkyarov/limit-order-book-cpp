#include "types.hpp"
#include "order.hpp"

struct PriceLevel {
    Price price;
    Quantity total_volume;

    Order* head;
    Order* tail;
};