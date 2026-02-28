#include "include/lob/order_book.hpp"

using namespace lob;

OrderBook::OrderBook(std::size_t pool_size)
    : pool_(pool_size) 
    {
        // OrderBook book(1'000'000); // test
    }