#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stock_order.h"

// Function prototypes
void show_main_menu(void);
void transaction_view(void);
void transaction_list(void);
void clear_screen(void);
void wait_for_enter(void);

int main(void) {
    int choice;
    int running = 1;

    while (running) {
        show_main_menu();

        if (scanf("%d", &choice) != 1) {
            // Clear invalid input
            while (getchar() != '\n');
            printf("\nInvalid input. Please enter a number.\n");
            wait_for_enter();
            continue;
        }

        // Clear remaining newline
        while (getchar() != '\n');

        clear_screen();

        switch (choice) {
            case 1:
                transaction_view();
                break;
            case 2:
                transaction_list();
                break;
            case 0:
                running = 0;
                clear_screen();
                break;
            default:
                printf("\nInvalid option. Please select 1, 2, or 0.\n");
                wait_for_enter();
        }
    }

    return 0;
}

void show_main_menu(void) {
    clear_screen();
    printf("=====================================\n");
    printf("     STOCK TRADING SYSTEM MENU\n");
    printf("=====================================\n\n");
    printf("1. Transaction view\n");
    printf("2. Transaction list\n");
    printf("0. Exit\n\n");
    printf("Select an option: ");
}

void transaction_view(void) {
    clear_screen();
    printf("=====================================\n");
    printf("         TRANSACTION VIEW\n");
    printf("=====================================\n\n");

    // TODO: Implement transaction view functionality
    printf("Transaction view - Coming soon\n\n");

    wait_for_enter();
}

void transaction_list(void) {
    clear_screen();
    printf("=====================================\n");
    printf("         TRANSACTION LIST\n");
    printf("=====================================\n\n");

    // TODO: Implement transaction list functionality
    printf("Transaction list - Coming soon\n\n");

    wait_for_enter();
}

void clear_screen(void) {
    // ANSI escape code to clear screen
    printf("\033[2J\033[H");
    fflush(stdout);
}

void wait_for_enter(void) {
    printf("Press Enter to continue...");
    getchar();
}
