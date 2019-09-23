#include <immintrin.h>
#include <stdint.h>
#include "checker.h"
#include "prob_desc.h"

#ifdef VECT
#include "popcount256_16.h"

void init_sh_all(__m256i probes_a_all[NB_PR], __m256i probes_b_all[NB_PR])
{
    uint64_t i;
    for (i = 0; i < NB_PR; i++) {
        probes_a_all[i] = _mm256_loadu_si256((__m256i*)(probes_sh_a[i]));
        probes_b_all[i] = _mm256_loadu_si256((__m256i*)(probes_sh_b[i]));
    }
}

void init_sh_curr(__m256i *probes_sh_curr, __m256i probes_sh_all[NB_PR],
        uint64_t *combination, uint64_t k)
{
    *probes_sh_curr = _mm256_setzero_si256();
    uint64_t i, j;
    for (i = 0; i < k; i++) {
        j = combination[i];
        *probes_sh_curr = _mm256_xor_si256(*probes_sh_curr, probes_sh_all[j]);
    }
}

void init_r_curr(uint64_t *probes_r_curr, uint64_t *combination, uint64_t k)
{
    uint64_t i, j;
    *probes_r_curr = 0;
    for (i = 0; i < k; i++) {
        j = combination[i];
        *probes_r_curr ^= probes_r[j];
    }
}


int check_attack_ni(uint64_t nb_probes, uint64_t r_sum, __m256i sh_sum_a,
        __m256i sh_sum_b)
{
    uint64_t nb_rand = __builtin_popcountl(r_sum);
    uint64_t nb_a;
    uint64_t nb_b;

    /* Is there enough probes left to cancel out remaining random? */
    if (nb_rand + nb_probes <= D) {
        /* Is the minimum number of probes needed to build an attack on the
         * a_i strictly greater than D? */
        nb_a = popcount256_16(sh_sum_a);
        if (nb_rand + nb_probes + (NB_SH - nb_a) <= D) {
            return 1;
        }

        /* Same on the b_j */
        nb_b = popcount256_16(sh_sum_b);
        if (nb_rand + nb_probes + (NB_SH - nb_b) <= D) {
            return 1;
        }
    }
    return 0;
}

int check_attack_sni(uint64_t nb_probes, uint64_t nb_internal, uint64_t r_sum,
        __m256i sh_sum_a, __m256i sh_sum_b)
{
    uint64_t nb_rand = __builtin_popcountl(r_sum);
    uint64_t nb_a;
    uint64_t nb_b;

    /* Is there enough probes left to cancel out remaining random? */
    if (nb_rand + nb_probes <= D) {
        /* Probes used to eliminate random are internal probes */
        nb_internal += nb_rand;

        nb_a = popcount256_16(sh_sum_a);
        if (nb_a > nb_internal) {
            return 1;
        }

        nb_b = popcount256_16(sh_sum_b);
        if (nb_b > nb_internal) {
            return 1;
        }
    }
    return 0;
}

#else

void probes_sh_xor(uint64_t probes_sh_dst[NB_SH][SIZE_SH],
        uint64_t probes_sh_src[NB_SH][SIZE_SH])
{
    uint64_t i, j;
    for (i = 0; i < NB_SH; i++) {
        for (j = 0; j < SIZE_SH; j++) {
            probes_sh_dst[i][j] ^= probes_sh_src[i][j];
        }
    }
}

void probes_r_xor(uint64_t probes_r_dst[SIZE_R], uint64_t probes_r_src[SIZE_R])
{
    uint64_t i;
    for (i = 0; i < SIZE_R; i++) {
        probes_r_dst[i] ^= probes_r_src[i];
    }
}

uint64_t probes_r_count(uint64_t probes_r[SIZE_R])
{
    uint64_t i, c = 0;
    for (i = 0; i < SIZE_R; i++) {
        c += __builtin_popcountl(probes_r[i]);
    }
    return c;
}

uint64_t probes_sh_count(uint64_t probes_sh[NB_SH][SIZE_SH])
{
    uint64_t i, j, c = 0;
    for (i = 0; i < NB_SH; i++) {
        for (j = 0; j < SIZE_SH; j++) {
            if (probes_sh[i][j] != 0) {
                c += 1;
                break;
            }
        }
    }
    return c;
}

void init_sh_curr(uint64_t probes_sh_curr[NB_SH][SIZE_SH],
        uint64_t probes_sh_all[NB_PR][NB_SH][SIZE_SH], uint64_t *combination,
        uint64_t k)
{
    uint64_t i, j;

    /* Zero init */
    for (i = 0; i < NB_SH; i++) {
        for (j = 0; j < SIZE_SH; j++) {
            probes_sh_curr[i][j] = 0;
        }
    }

    /* Adding the right probes */
    for (i = 0; i < k; i++) {
            j = combination[i];
            probes_sh_xor(probes_sh_curr, probes_sh_all[j]);
    }
}

void init_r_curr(uint64_t probes_r_curr[SIZE_R], uint64_t *combination, uint64_t k)
{
    uint64_t i, j;

    for (i = 0; i < SIZE_R; i++) {
        probes_r_curr[i] = 0;
    }

    for (i = 0; i < k; i++) {
        j = combination[i];
        probes_r_xor(probes_r_curr, probes_r[j]);
    }
}

int check_attack_ni(uint64_t nb_probes, uint64_t r_sum[SIZE_R],
        uint64_t sh_sum_a[NB_SH][SIZE_SH], uint64_t sh_sum_b[NB_SH][SIZE_SH])
{
    uint64_t nb_rand = probes_r_count(r_sum);
    uint64_t nb_a;
    uint64_t nb_b;

    /* Is there enough probes left to cancel out remaining random? */
    if (nb_rand + nb_probes <= D) {
        /* Is the minimum number of probes needed to build an attack on the
         * a_i strictly greater than D? */
        nb_a = probes_sh_count(sh_sum_a);
        if (nb_rand + nb_probes + (NB_SH - nb_a) <= D) {
            return 1;
        }

        /* Same on the b_j */
        nb_b = probes_sh_count(sh_sum_b);
        if (nb_rand + nb_probes + (NB_SH - nb_b) <= D) {
            return 1;
        }
    }
    return 0;
}

int check_attack_sni(uint64_t nb_probes, uint64_t nb_internal,
        uint64_t r_sum[SIZE_R], uint64_t sh_sum_a[NB_SH][SIZE_SH],
        uint64_t sh_sum_b[NB_SH][SIZE_SH])
{
    uint64_t nb_rand = probes_r_count(r_sum);
    uint64_t nb_a;
    uint64_t nb_b;

    /* Is there enough probes left to cancel out remaining random? */
    if (nb_rand + nb_probes <= D) {
        /* Probes used to eliminate random are internal probes */
        nb_internal += nb_rand;

        nb_a = probes_sh_count(sh_sum_a);
        if (nb_a > nb_internal) {
            return 1;
        }

        nb_b = probes_sh_count(sh_sum_b);
        if (nb_b > nb_internal) {
            return 1;
        }
    }
    return 0;
}
#endif /* VECT */
