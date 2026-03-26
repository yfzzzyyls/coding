#include <stdio.h>

int maxProfit(int* prices, int pricesSize) {
    int buy_price = prices[0];
    int profit = 0;
    
    for (int i = 1; i < pricesSize; i++) {
        if (prices[i] < buy_price) {
            buy_price = prices[i];
        } else {
            int current_profit = prices[i] - buy_price;
            if (current_profit > profit) {
                profit = current_profit;
            }
        }
    }

    return profit;
}

int main(void) {
    int prices[] = {7, 1, 5, 3, 6, 4};
    int pricesSize = sizeof(prices) / sizeof(prices[0]);

    int profit = maxProfit(prices, pricesSize);
    printf("[Profit]: %d\n", profit);
    return 0;
}
