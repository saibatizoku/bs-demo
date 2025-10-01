#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock_order.h"

// Function prototypes
void show_main_menu(void);
void new_transaction(void);
void transaction_list(void);
void clear_screen(void);
void wait_for_enter(void);
int check_exit(const char *input);
int str_case_cmp(const char *s1, const char *s2);
void str_to_upper(char *str);
int compare_orders_desc(const void *a, const void *b);
int save_transaction(const StockOrder *order);
int load_transactions(StockOrder *orders, int max_orders);
void initialize_data_file(void);

int main(void) {
    int choice;
    int running = 1;

    while (running) {
        if (running) {  /* Only show menu if still running */
            show_main_menu();
        }

        if (scanf("%d", &choice) != 1) {
            /* Clear invalid input */
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            printf("\nInvalid input. Please enter a number.\n");
            wait_for_enter();
            continue;
        }

        /* Clear remaining newline */
        {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
        }

        if (choice == 0) {
            /* Exit immediately - no function calls, no cleanup */
            break;  /* Just break out of loop */
        }

        switch (choice) {
            case 1:
                clear_screen();
                new_transaction();
                break;
            case 2:
                clear_screen();
                transaction_list();
                break;
            default:
                clear_screen();
                printf("\nInvalid option. Please select 1, 2, or 0.\n");
                wait_for_enter();
                break;
        }
    }

    /* Direct exit - no function calls */
    return 0;
}

void show_main_menu(void) {
    clear_screen();
    printf("=====================================\n");
    printf("     STOCK TRADING SYSTEM MENU\n");
    printf("=====================================\n\n");
    printf("1. New transaction\n");
    printf("2. Transaction list\n");
    printf("0. Exit\n\n");
    printf("Select an option: ");
}

void new_transaction(void) {
    StockOrder order;
    char input[256];
    char action_input[16];
    char type_input[16];
    time_t current_time;
    struct tm *local_time;
    int valid_input;
    long account_num;

    /* Initialize the entire order structure */
    memset(&order, 0, sizeof(StockOrder));

    /* Get current Unix timestamp */
    time(&current_time);
    order.timestamp = current_time;

    /* Display current date/time */
    local_time = localtime(&current_time);

    printf("=====================================\n");
    printf("         NEW TRANSACTION\n");
    printf("=====================================\n\n");
    if (local_time != NULL) {
        printf("Date: %02d/%02d/%04d Time: %02d:%02d\n\n",
               local_time->tm_mday,
               local_time->tm_mon + 1,
               local_time->tm_year + 1900,
               local_time->tm_hour,
               local_time->tm_min);
    }
    printf("Type 'exit' at any prompt to cancel\n\n");

    /* Customer Account Number */
    do {
        valid_input = 1;
        printf("Customer Account No. (6 digits): ");
        if (fgets(input, sizeof(input), stdin) == NULL) return;
        input[strcspn(input, "\n")] = 0;
        if (check_exit(input)) return;

        account_num = atol(input);
        if (account_num < 100000 || account_num > 999999) {
            printf("Error: Account number must be 6 digits\n");
            valid_input = 0;
        } else {
            order.customer_account_no = (uint32_t)account_num;
        }
    } while (!valid_input);

    /* Broker ID */
    do {
        valid_input = 1;
        printf("Broker ID (3-15 chars): ");
        if (fgets(input, sizeof(input), stdin) == NULL) return;
        input[strcspn(input, "\n")] = 0;
        if (check_exit(input)) return;

        if (strlen(input) < 3 || strlen(input) > 15) {
            printf("Error: Broker ID must be between 3 and 15 characters\n");
            valid_input = 0;
        } else {
            memset(order.broker_id, 0, sizeof(order.broker_id));
            strncpy(order.broker_id, input, sizeof(order.broker_id) - 1);
        }
    } while (!valid_input);

    /* Order Action */
    do {
        valid_input = 1;
        printf("Order Action (BUY/SELL): ");
        if (fgets(action_input, sizeof(action_input), stdin) == NULL) return;
        action_input[strcspn(action_input, "\n")] = 0;
        if (check_exit(action_input)) return;
        str_to_upper(action_input);

        if (strcmp(action_input, "BUY") == 0) {
            order.action = ORDER_ACTION_BUY;
        } else if (strcmp(action_input, "SELL") == 0) {
            order.action = ORDER_ACTION_SELL;
        } else {
            printf("Error: Must be BUY or SELL\n");
            valid_input = 0;
        }
    } while (!valid_input);

    /* Quantity */
    do {
        valid_input = 1;
        printf("Quantity (1-9999 shares): ");
        if (fgets(input, sizeof(input), stdin) == NULL) return;
        input[strcspn(input, "\n")] = 0;
        if (check_exit(input)) return;

        order.quantity = (uint32_t)atoi(input);
        if (order.quantity < 1 || order.quantity > 9999) {
            printf("Error: Quantity must be between 1 and 9999\n");
            valid_input = 0;
        }
    } while (!valid_input);

    /* Price */
    do {
        valid_input = 1;
        printf("Price ($0.01-$9999.99): ");
        if (fgets(input, sizeof(input), stdin) == NULL) return;
        input[strcspn(input, "\n")] = 0;
        if (check_exit(input)) return;

        {
            int whole_part = 0;
            int decimal_part = 0;
            int i = 0;
            int divisor = 1;

            /* Parse whole part */
            while (input[i] != '\0' && input[i] >= '0' && input[i] <= '9') {
                whole_part = whole_part * 10 + (input[i] - '0');
                i++;
            }

            /* Parse decimal part */
            if (input[i] == '.') {
                i++;
                while (input[i] != '\0' && input[i] >= '0' && input[i] <= '9') {
                    decimal_part = decimal_part * 10 + (input[i] - '0');
                    divisor *= 10;
                    i++;
                    if (divisor > 100) break;  /* Limit to 2 decimal places */
                }
            }

            order.price = (double)whole_part + (double)decimal_part / (double)divisor;

            if (order.price < 0.01 || order.price > 9999.99) {
                printf("Error: Price must be between $0.01 and $9999.99\n");
                valid_input = 0;
            }
        }
    } while (!valid_input);

    /* Ticker */
    do {
        valid_input = 1;
        printf("Ticker Symbol (1-7 chars): ");
        if (fgets(input, sizeof(input), stdin) == NULL) return;
        input[strcspn(input, "\n")] = 0;
        if (check_exit(input)) return;

        /* Convert ticker to uppercase */
        str_to_upper(input);

        if (strlen(input) < 1 || strlen(input) > 7) {
            printf("Error: Ticker must be between 1 and 7 characters\n");
            valid_input = 0;
        } else {
            /* Check if ticker contains only letters */
            int i;
            for (i = 0; input[i]; i++) {
                if (input[i] < 'A' || input[i] > 'Z') {
                    printf("Error: Ticker must contain only letters\n");
                    valid_input = 0;
                    break;
                }
            }
            if (valid_input) {
                memset(order.ticker, 0, sizeof(order.ticker));
                strncpy(order.ticker, input, sizeof(order.ticker) - 1);
            }
        }
    } while (!valid_input);

    /* Order Type */
    do {
        valid_input = 1;
        printf("Order Type (MARKET/LIMIT): ");
        if (fgets(type_input, sizeof(type_input), stdin) == NULL) return;
        type_input[strcspn(type_input, "\n")] = 0;
        if (check_exit(type_input)) return;
        str_to_upper(type_input);

        if (strcmp(type_input, "LIMIT") == 0) {
            order.order_type = ORDER_TYPE_LIMIT;
        } else if (strcmp(type_input, "MARKET") == 0) {
            order.order_type = ORDER_TYPE_MARKET;
        } else {
            printf("Error: Must be MARKET or LIMIT\n");
            valid_input = 0;
        }
    } while (!valid_input);

    /* Save transaction to file */
    if (save_transaction(&order)) {
        printf("\nTransaction saved successfully!\n\n");
    } else {
        printf("\nError: Could not save transaction to file.\n\n");
    }
    wait_for_enter();
}

int check_exit(const char *input) {
    return str_case_cmp(input, "exit") == 0;
}

int str_case_cmp(const char *s1, const char *s2) {
    while (*s1 && *s2) {
        char c1 = (*s1 >= 'A' && *s1 <= 'Z') ? *s1 + 32 : *s1;
        char c2 = (*s2 >= 'A' && *s2 <= 'Z') ? *s2 + 32 : *s2;
        if (c1 != c2) return c1 - c2;
        s1++;
        s2++;
    }
    return *s1 - *s2;
}

void str_to_upper(char *str) {
    while (*str) {
        if (*str >= 'a' && *str <= 'z') {
            *str = *str - 32;
        }
        str++;
    }
}

int compare_orders_desc(const void *a, const void *b) {
    const StockOrder *order_a = (const StockOrder *)a;
    const StockOrder *order_b = (const StockOrder *)b;

    /* Compare Unix timestamps - descending order */
    if (order_b->timestamp > order_a->timestamp) {
        return 1;
    } else if (order_b->timestamp < order_a->timestamp) {
        return -1;
    }
    return 0;
}

void transaction_list(void) {
    #define MAX_DISPLAY_ORDERS 50  /* Reduced from 100 to save memory */
    #define ORDERS_PER_PAGE 10
    static StockOrder orders[MAX_DISPLAY_ORDERS];  /* Use static to avoid stack overflow */
    int count, i;
    int current_page = 0;
    int total_pages;
    char navigation[10];
    int viewing = 1;

    /* Load transactions from file */
    count = load_transactions(orders, MAX_DISPLAY_ORDERS);

    if (count == 0) {
        clear_screen();
        printf("No transactions found.\n");
        wait_for_enter();
        return;
    }

    /* Ensure count doesn't exceed array bounds */
    if (count > MAX_DISPLAY_ORDERS) {
        count = MAX_DISPLAY_ORDERS;
    }

    /* Sort orders by date/time descending */
    qsort(orders, count, sizeof(StockOrder), compare_orders_desc);

    /* Calculate total pages */
    total_pages = (count + ORDERS_PER_PAGE - 1) / ORDERS_PER_PAGE;

    while (viewing) {
        int start_index = current_page * ORDERS_PER_PAGE;
        int end_index = start_index + ORDERS_PER_PAGE;
        if (end_index > count) {
            end_index = count;
        }

        clear_screen();

        printf("===============================================================================\n");
        printf("                     TRANSACTION LIST - Page %d of %d\n", current_page + 1, total_pages);
        printf("===============================================================================\n\n");

        /* Table header with fixed widths */
        printf("%-8s %-16s %-10s %-6s %-5s %-9s %-7s %-6s\n",
               "Acct#", "Timestamp", "Broker", "Action", "Qty", "Price", "Ticker", "Type");
        printf("-------------------------------------------------------------------------------\n");

        /* Display orders for current page */
        for (i = start_index; i < end_index; i++) {
            char timestamp_str[20];
            struct tm *tm_info;

            /* Convert Unix timestamp to tm structure */
            tm_info = localtime(&orders[i].timestamp);
            if (tm_info != NULL) {
                sprintf(timestamp_str, "%02d/%02d/%02d %02d:%02d",
                        tm_info->tm_mon + 1,
                        tm_info->tm_mday,
                        tm_info->tm_year % 100,
                        tm_info->tm_hour,
                        tm_info->tm_min);
            } else {
                /* Fallback if localtime fails */
                sprintf(timestamp_str, "UNIX:%ld", (long)orders[i].timestamp);
            }

            printf("%-8lu %-16s %-10.10s %-6s %-5lu $%-8.2f %-7.7s %-6s\n",
                (unsigned long)orders[i].customer_account_no,
                timestamp_str,
                orders[i].broker_id,
                orders[i].action == ORDER_ACTION_BUY ? "BUY" : "SELL",
                (unsigned long)orders[i].quantity,
                orders[i].price,
                orders[i].ticker,
                orders[i].order_type == ORDER_TYPE_LIMIT ? "LIMIT" : "MARKET");
        }

        printf("\n-------------------------------------------------------------------------------\n");
        printf("Total transactions: %d\n", count);
        printf("Commands: [N]ext page, [P]revious page, [M]ain menu\n");
        printf("Enter command: ");

        if (fgets(navigation, sizeof(navigation), stdin) == NULL) {
            viewing = 0;
            continue;
        }
        navigation[strcspn(navigation, "\n")] = 0;
        str_to_upper(navigation);

        if (navigation[0] == 'N') {
            if (current_page < total_pages - 1) {
                current_page++;
            } else {
                printf("Already on last page. Press Enter to continue...");
                getchar();
            }
        } else if (navigation[0] == 'P') {
            if (current_page > 0) {
                current_page--;
            } else {
                printf("Already on first page. Press Enter to continue...");
                getchar();
            }
        } else if (navigation[0] == 'M') {
            viewing = 0;
        } else {
            printf("Invalid command. Press Enter to continue...");
            getchar();
        }
    }
}

void clear_screen(void) {
    /* Simple newlines for Amiga compatibility - avoids crashes */
    int i;
    for (i = 0; i < 25; i++) {
        printf("\n");
    }
    fflush(stdout);
}

void wait_for_enter(void) {
    int c;
    printf("Press Enter to continue...");
    fflush(stdout);
    /* Consume any character until newline */
    while ((c = getchar()) != '\n' && c != EOF);
}

int save_transaction(const StockOrder *order) {
    FILE *fp;

    fp = fopen("transactions.dat", "ab");
    if (fp == NULL) {
        return 0;
    }

    fwrite(order, sizeof(StockOrder), 1, fp);
    fclose(fp);
    return 1;
}

int load_transactions(StockOrder *orders, int max_orders) {
    FILE *fp;
    int count = 0;

    fp = fopen("transactions.dat", "rb");
    if (fp == NULL) {
        initialize_data_file();
        fp = fopen("transactions.dat", "rb");
        if (fp == NULL) {
            return 0;
        }
    }

    while (count < max_orders && fread(&orders[count], sizeof(StockOrder), 1, fp) == 1) {
        count++;
    }

    fclose(fp);
    return count;
}

void initialize_data_file(void) {
    FILE *fp;
    static StockOrder orders[10];  /* Use static to avoid stack issues */
    int i;

    /* Check if file already exists */
    fp = fopen("transactions.dat", "rb");
    if (fp != NULL) {
        fclose(fp);
        return;
    }

    /* Create file with initial data */
    fp = fopen("transactions.dat", "wb");
    if (fp == NULL) {
        return;
    }

    /* Initialize all orders to 0 first */
    memset(orders, 0, sizeof(orders));

    /* Base timestamp for Oct 1, 1988 00:00:00 UTC */
    time_t base_timestamp = 591667200L;  /* Unix timestamp for Oct 1, 1988 */

    /* Order 1 - Oct 3, 1988 09:30 - GM (General Motors) */
    orders[0].customer_account_no = 123456;
    orders[0].timestamp = base_timestamp + (2 * 86400) + (9 * 3600) + (30 * 60);
    strcpy(orders[0].broker_id, "MER");  /* Merrill Lynch */
    orders[0].action = ORDER_ACTION_BUY;
    orders[0].quantity = 100;
    orders[0].price = 84.25;  /* Realistic GM price in 1988 */
    strcpy(orders[0].ticker, "GM");
    orders[0].order_type = ORDER_TYPE_LIMIT;

    /* Order 2 - Oct 5, 1988 14:30 - IBM */
    orders[1].customer_account_no = 234567;
    orders[1].timestamp = base_timestamp + (4 * 86400) + (14 * 3600) + (30 * 60);
    strcpy(orders[1].broker_id, "DLJ");  /* Donaldson, Lufkin & Jenrette */
    orders[1].action = ORDER_ACTION_SELL;
    orders[1].quantity = 50;
    orders[1].price = 129.50;  /* IBM was around $125-135 in Oct 1988 */
    strcpy(orders[1].ticker, "IBM");
    orders[1].order_type = ORDER_TYPE_MARKET;

    /* Order 3 - Oct 7, 1988 10:15 - GE (General Electric) */
    orders[2].customer_account_no = 345678;
    orders[2].timestamp = base_timestamp + (6 * 86400) + (10 * 3600) + (15 * 60);
    strcpy(orders[2].broker_id, "GS");  /* Goldman Sachs */
    orders[2].action = ORDER_ACTION_BUY;
    orders[2].quantity = 200;
    orders[2].price = 44.75;  /* GE price in 1988 */
    strcpy(orders[2].ticker, "GE");
    orders[2].order_type = ORDER_TYPE_LIMIT;

    /* Order 4 - Oct 11, 1988 11:00 - XON (Exxon) */
    orders[3].customer_account_no = 456789;
    orders[3].timestamp = base_timestamp + (10 * 86400) + (11 * 3600);
    strcpy(orders[3].broker_id, "MS");  /* Morgan Stanley */
    orders[3].action = ORDER_ACTION_BUY;
    orders[3].quantity = 150;
    orders[3].price = 45.50;  /* Exxon price in 1988 */
    strcpy(orders[3].ticker, "XON");
    orders[3].order_type = ORDER_TYPE_MARKET;

    /* Order 5 - Oct 14, 1988 15:45 - KO (Coca-Cola) */
    orders[4].customer_account_no = 567890;
    orders[4].timestamp = base_timestamp + (13 * 86400) + (15 * 3600) + (45 * 60);
    strcpy(orders[4].broker_id, "BSC");  /* Bear Stearns */
    orders[4].action = ORDER_ACTION_BUY;
    orders[4].quantity = 300;
    orders[4].price = 42.25;  /* KO price in 1988 */
    strcpy(orders[4].ticker, "KO");
    orders[4].order_type = ORDER_TYPE_LIMIT;

    /* Order 6 - Oct 18, 1988 10:20 - F (Ford) */
    orders[5].customer_account_no = 678901;
    orders[5].timestamp = base_timestamp + (17 * 86400) + (10 * 3600) + (20 * 60);
    strcpy(orders[5].broker_id, "PWJ");  /* PaineWebber */
    orders[5].action = ORDER_ACTION_BUY;
    orders[5].quantity = 200;
    orders[5].price = 52.75;  /* Ford price in 1988 */
    strcpy(orders[5].ticker, "F");
    orders[5].order_type = ORDER_TYPE_MARKET;

    /* Order 7 - Oct 20, 1988 13:10 - T (AT&T) */
    orders[6].customer_account_no = 789012;
    orders[6].timestamp = base_timestamp + (19 * 86400) + (13 * 3600) + (10 * 60);
    strcpy(orders[6].broker_id, "LEH");  /* Lehman Brothers */
    orders[6].action = ORDER_ACTION_SELL;
    orders[6].quantity = 100;
    orders[6].price = 28.50;  /* AT&T price in 1988 */
    strcpy(orders[6].ticker, "T");
    orders[6].order_type = ORDER_TYPE_LIMIT;

    /* Order 8 - Oct 24, 1988 09:30 - MRK (Merck) */
    orders[7].customer_account_no = 890123;
    orders[7].timestamp = base_timestamp + (23 * 86400) + (9 * 3600) + (30 * 60);
    strcpy(orders[7].broker_id, "SLB");  /* Salomon Brothers */
    orders[7].action = ORDER_ACTION_BUY;
    orders[7].quantity = 75;
    orders[7].price = 58.25;  /* Merck price in 1988 */
    strcpy(orders[7].ticker, "MRK");
    orders[7].order_type = ORDER_TYPE_MARKET;

    /* Order 9 - Oct 26, 1988 16:00 - PG (Procter & Gamble) */
    orders[8].customer_account_no = 901234;
    orders[8].timestamp = base_timestamp + (25 * 86400) + (16 * 3600);
    strcpy(orders[8].broker_id, "DWR");  /* Dean Witter Reynolds */
    orders[8].action = ORDER_ACTION_SELL;
    orders[8].quantity = 125;
    orders[8].price = 89.75;  /* P&G price in 1988 */
    strcpy(orders[8].ticker, "PG");
    orders[8].order_type = ORDER_TYPE_LIMIT;

    /* Order 10 - Oct 28, 1988 11:55 - GE (General Electric) */
    orders[9].customer_account_no = 112345;
    orders[9].timestamp = base_timestamp + (27 * 86400) + (11 * 3600) + (55 * 60);
    strcpy(orders[9].broker_id, "EFH");  /* E.F. Hutton */
    orders[9].action = ORDER_ACTION_BUY;
    orders[9].quantity = 150;
    orders[9].price = 44.50;  /* GE price in 1988 */
    strcpy(orders[9].ticker, "GE");
    orders[9].order_type = ORDER_TYPE_MARKET;

    /* Write all orders to file */
    fwrite(orders, sizeof(StockOrder), 10, fp);
    fclose(fp);
}
