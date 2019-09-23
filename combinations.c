#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "combinations.h"

uint64_t nCr(uint64_t n, uint64_t k)
{
    uint64_t i, j;
    uint64_t row[n + 1];

    k = (n - k < k) ? (n - k) : k;

    // initialize by the first row
    row[0] = 1; // this is the value of C(0, 0)

    for(i = 1; i <= n; i++) {
        row[i] = 0;
        for(j = i; j > 0; j--) {
             // from the recurrence C(n, r) = C(n - 1, r - 1) + C(n - 1, r)
             row[j] += row[j - 1];
        }
    }

    return row[k];
}

void print_combination(struct comb_t comb_struct)
{
    uint64_t i;
    for (i = 0; i < comb_struct.k; i++) {
        printf("%lu ", comb_struct.combination[i]);
    }
    printf("\n");
//    printf("%d %d\n", combination[k], combination[k + 1]);
}

void init_combination(struct comb_t *comb_struct, uint64_t *combination,  uint64_t k, uint64_t n)
{
    uint64_t j;
    for (j = 0; j < k; j++) {
        combination[j] = j;
    }
    combination[k] = n;

    comb_struct->combination = combination;
    comb_struct->done = 0;
    comb_struct->k = k;
    comb_struct->n = n;
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
void next_combination(struct comb_t *comb_struct, struct comb_diff_t *comb_diff)
{
    uint64_t j;
    uint64_t *comb = comb_struct->combination;

    if (comb_struct->k % 2) {
        // k is odd
        // R3 : easy case
        if (comb[0] + 1 < comb[1]) {
            comb_diff->to_del = comb[0];
            comb_diff->to_add = comb[0] + 1;
            comb[0]++;
            return;
        }

        j = 1;
    } else {
        // k is even
        // R3 : easy case
        if (comb[0] > 0) {
            comb_diff->to_del = comb[0];
            comb_diff->to_add = comb[0] - 1;
            comb[0]--;
            return;
        }

        j = 1;
        // R5 : try to increase comb[j]
        // at this point comb[j-1] = j-1
        if (comb[j] + 1 < comb[j+1]) {
            comb_diff->to_del = comb[j - 1];
            comb_diff->to_add = comb[j] + 1;
            comb[j-1] = comb[j];
            comb[j]++;
            return;
        }
        j++;

    }

    while (j < comb_struct->k) {
        // R4 : try to decrease comb[j]
        // at this point comb[j] = comb[j-1] + 1
        if (comb[j] > j) {
            comb_diff->to_del = comb[j];
            comb_diff->to_add = j - 1;
            comb[j] = comb[j-1];
            comb[j-1] = j-1;
            break;
        }
        j++;

        // R5 : try to increase comb[j]
        // at this point comb[j-1] = j-1
        if (comb[j] + 1 < comb[j+1]) {
            comb_diff->to_del = comb[j - 1];
            comb_diff->to_add = comb[j] + 1;
            comb[j-1] = comb[j];
            comb[j]++;
            break;
        }
        j++;
    }
    if (j == comb_struct->k) {
        comb_struct->done = 1;
        return; // No more comb
    }
}

void unrank(uint64_t rank, struct comb_t comb_struct)
{
    int64_t i;
    uint64_t *comb = comb_struct.combination;
    uint64_t tmp;
    uint64_t k = comb_struct.k;

    if (rank >= nCr(comb_struct.n, k)) {
        rank = nCr(comb_struct.n, k) - 1;
    }

    for (i = k - 1; i >= 0; i--) {
        comb[i] = i;
        tmp = nCr(comb[i] + 1, i + 1) - 1;
        while (tmp < rank) {
            comb[i] += 1;
            tmp = nCr(comb[i] + 1, i + 1) - 1;
        }
        rank = tmp - rank;
    }

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
