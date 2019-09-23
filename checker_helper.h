#ifndef CHECKER_HELPER_H
#define CHECKER_HELPER_H
#include "prob_desc.h"

#ifdef VECT

void init_sh_all(__m256i probes_a_all[NB_PR], __m256i probes_b_all[NB_PR]);
void init_sh_curr(__m256i *probes_sh_curr, __m256i probes_sh_all[NB_PR],
        uint64_t *combination, uint64_t k);
void init_r_curr(uint64_t *probes_r_curr, uint64_t *combination, uint64_t k);
int check_attack_ni(uint64_t nb_probes, uint64_t r_sum, __m256i sh_sum_a,
        __m256i sh_sum_b);
int check_attack_sni(uint64_t nb_probes, uint64_t nb_internal, uint64_t r_sum,
        __m256i sh_sum_a, __m256i sh_sum_b);

#else

void init_sh_curr(uint64_t probes_sh_curr[NB_SH][SIZE_SH],
        uint64_t probes_sh_all[NB_PR][NB_SH][SIZE_SH], uint64_t *combination,
        uint64_t k);
void init_r_curr(uint64_t probes_r_curr[SIZE_R], uint64_t *combination, uint64_t k);

int check_attack_ni(uint64_t nb_probes, uint64_t r_sum[SIZE_R],
        uint64_t sh_sum_a[NB_SH][SIZE_SH], uint64_t sh_sum_b[NB_SH][SIZE_SH]);
int check_attack_sni(uint64_t nb_probes, uint64_t nb_internal,
        uint64_t r_sum[SIZE_R], uint64_t sh_sum_a[NB_SH][SIZE_SH],
        uint64_t sh_sum_b[NB_SH][SIZE_SH]);
void probes_sh_xor(uint64_t probes_sh_dst[NB_SH][SIZE_SH],
        uint64_t probes_sh_src[NB_SH][SIZE_SH]);
void probes_r_xor(uint64_t probes_r_dst[SIZE_R], uint64_t probes_r_src[SIZE_R]);
uint64_t probes_sh_count(uint64_t probes_sh[NB_SH][SIZE_SH]);
uint64_t probes_r_count(uint64_t probes_r[SIZE_R]);

#endif /* VECT */
#endif /* CHECKER_HELPER_H */
