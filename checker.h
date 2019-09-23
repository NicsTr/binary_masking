#ifndef CHECKER_H
#define CHECKER_H

#include <stdint.h>

#include "combinations.h"

int check_full(int check_sni);
int check_partial(struct comb_t comb_struct, uint64_t nb, int check_sni);

#endif /* CHECKER_H */
