#ifndef STOCK_ORDER_H
#define STOCK_ORDER_H

#include <stdint.h>
#include <time.h>

// Enum for order action (BUY/SELL)
typedef enum {
    ORDER_ACTION_BUY,
    ORDER_ACTION_SELL
} OrderAction;

// Enum for order type (MARKET/LIMIT/STOP)
typedef enum {
    ORDER_TYPE_MARKET,
    ORDER_TYPE_LIMIT
} OrderType;

// Stock order structure
typedef struct {
    uint32_t customer_account_no;  // Customer account number
    time_t timestamp;               // Unix timestamp of the order
    char broker_id[16];             // Broker ID (e.g., "JDS")
    OrderAction action;             // BUY or SELL
    uint32_t quantity;              // Number of shares
    double price;                   // Price per share in dollars
    char ticker[8];                 // Stock ticker symbol (e.g., "GM")
    OrderType order_type;           // LIMIT, or MARKET
    int confirmed;                  // 0 = unconfirmed, 1 = confirmed
} StockOrder;

#endif // STOCK_ORDER_H
