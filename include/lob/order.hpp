#include "types.hpp"
#include "side.hpp"

struct Order {
    OrderId   id;
    Price     price;
    Quantity  qty;
    Quantity  remaining;
    Side      side;
    Timestamp ts;

    Order* next;   // intrusive linked list
    Order* prev;
};