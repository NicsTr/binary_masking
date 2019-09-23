#ifndef COMBINATION_H
#define COMBINATION_H

struct comb_t {
    uint64_t *combination;
    uint8_t done;
    uint64_t k;
    uint64_t n;
};

struct comb_diff_t {
    uint64_t to_add;
    uint64_t to_del;
};

void print_combination(struct comb_t comb_struct);
void init_combination(struct comb_t *comb_struct, uint64_t *combination,  uint64_t k, uint64_t n);
void next_combination(struct comb_t *comb_struct, struct comb_diff_t *comb_diff);
uint64_t nCr(uint64_t n, uint64_t k);
void unrank(uint64_t rank, struct comb_t comb_struct);

#endif /* COMBINATION_H */
