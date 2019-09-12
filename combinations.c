#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "combinations.h"

void print_combination(uint64_t *combination, uint64_t k)
{
    uint64_t i;
    for (i = 0; i < k; i++) {
        printf("%lu ", combination[i]);
    }
    printf("\n");
//    printf("%d %d\n", combination[k], combination[k + 1]);
}

void init_combination(uint64_t *combination,  uint64_t k, uint64_t n)
{
    uint64_t j;
    for (j = 0; j < k; j++) {
        combination[j] = j;
    }
    combination[k] = n;
}

/*
 *  Given the current `combination` state, and the parameter `k`,
 *  modify in place `combination` to the next combination of `k` among `n`,
 *  where `n` is defined during the initialization of the internal state
 *  `combination`.
 *
 *  Return the indexes that just changed or -1 if its the last combination
 *
 */
int32_t next_combination(uint64_t *combination, uint64_t k)
{
    uint64_t to_del, to_add;
    uint64_t j;

    if (k % 2) {
        // k is odd
        // R3 : easy case
        if (combination[0] + 1 < combination[1]) {
            to_del = combination[0];
            to_add = combination[0] + 1;
            combination[0]++;
            return (to_del << 16) | to_add;
        }

        j = 1;
    } else {
        // k is even
        // R3 : easy case
        if (combination[0] > 0) {
            to_del = combination[0];
            to_add = combination[0] - 1;
            combination[0]--;
            return (to_del << 16) | to_add;
        }

        j = 1;
        // R5 : try to increase combination[j]
        // at this point combination[j-1] = j-1
        if (combination[j] + 1 < combination[j+1]) {
            to_del = combination[j - 1];
            to_add = combination[j] + 1;
            combination[j-1] = combination[j];
            combination[j]++;
            return (to_del << 16) | to_add;
        }
        j++;

    }

    while (j < k) {
        // R4 : try to decrease combination[j]
        // at this point combination[j] = combination[j-1] + 1
        if (combination[j] > j) {
            to_del = combination[j];
            to_add = j - 1;
            combination[j] = combination[j-1];
            combination[j-1] = j-1;
            break;
        }
        j++;

        // R5 : try to increase combination[j]
        // at this point combination[j-1] = j-1
        if (combination[j] + 1 < combination[j+1]) {
            to_del = combination[j - 1];
            to_add = combination[j] + 1;
            combination[j-1] = combination[j];
            combination[j]++;
            break;
        }
        j++;
    }
    if (j == k) {
        return 0; // No more combination
    }
    return (to_del << 16) | to_add;

}

//void main() {
//    int32_t i;
//    uint64_t j;
//    int c = 0;
//    uint64_t k = 5;
//    uint64_t n = 8;
//    uint64_t comb[8] = {0};
//    init_combination(comb, k, n);
//
//    print_combination(comb, k);
//    i = next_combination(comb, k, n);
//    while (i != -1) {
//        print_combination(comb, k);
//        i = next_combination(comb, k, n);
//        c++;
//    }
//    printf("c = %d\n", c);
//
//}
