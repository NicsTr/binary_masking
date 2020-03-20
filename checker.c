#include <immintrin.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "combinations.h"
#include "checker_helper.h"
#include "checker.h"
#include "gray.h"

#ifdef VECT

int check_support(const struct comb_t comb_struct, const int nb_internal, __m256i probes_a_curr,
        __m256i probes_b_curr, uint64_t probes_r_curr, __m256i **probes_a_all,
        __m256i **probes_b_all, uint64_t *probes_r_all[NB_PR], int check_sni)
{

#ifdef GLITCH

    uint64_t *counters = malloc(comb_struct.k*sizeof(uint64_t));
    int *local_radices = malloc(comb_struct.k*sizeof(int));
    int *corres = malloc(comb_struct.k*sizeof(int));

    int nb_nontrivials = 0;
    uint64_t c = 0;
    uint64_t i;
    int to_incr, to_incr_bin, p;
    int attack = 0;

    for (i = 0; i < comb_struct.k; i++) {
        if (radices[comb_struct.combination[i]] > 1) {
            //printf("nb: %d\n", nb_nontrivials);
            counters[nb_nontrivials] = 1;
            local_radices[nb_nontrivials] = radices[comb_struct.combination[i]];
            corres[nb_nontrivials] = i;
            nb_nontrivials++;
        }
    }


    while ((to_incr = next_increment(&c, local_radices, nb_nontrivials)) != -1) {

        // Increment lower-level gray code
        to_incr_bin = next_increment_bin(&(counters[to_incr]), local_radices[to_incr]);
        p = comb_struct.combination[corres[to_incr]];
        probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[p][to_incr_bin]);
        probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[p][to_incr_bin]);
        probes_r_curr ^= probes_r_all[p][to_incr_bin];

        if (counters[to_incr] == 0) {
            to_incr_bin = next_increment_bin(&(counters[to_incr]), local_radices[to_incr]);
            p = comb_struct.combination[corres[to_incr]];
            probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[p][to_incr_bin]);
            probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[p][to_incr_bin]);
            probes_r_curr ^= probes_r_all[p][to_incr_bin];
        }


        if (check_sni) {
            attack |= check_attack_sni(comb_struct.k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
        } else {
            attack |= check_attack_ni(comb_struct.k, probes_r_curr, probes_a_curr, probes_b_curr);
        }

    }

    free(counters);
    free(local_radices);
    free(corres);

    return attack;
#endif

    if (check_sni) {
        return check_attack_sni(comb_struct.k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
    } else {
        return check_attack_ni(comb_struct.k, probes_r_curr, probes_a_curr, probes_b_curr);
    }

}

int next_support(struct comb_t *comb_struct, struct comb_diff_t *comb_diff,
        __m256i *probes_a_curr, __m256i *probes_b_curr, uint64_t *probes_r_curr,
        __m256i *probes_a_all[NB_PR],  __m256i *probes_b_all[NB_PR], uint64_t *probes_r_all[NB_PR],
        uint64_t *nb_internal, int check_sni)
{
    next_combination(comb_struct, comb_diff);
    if (comb_struct->done) return -1;

    // Adjust the number of internal probes
    if (check_sni) {
        if (comb_diff->to_del < NB_INT) nb_internal--;
        if (comb_diff->to_add < NB_INT) nb_internal++;
    }

    *probes_a_curr = _mm256_xor_si256(*probes_a_curr, probes_a_all[comb_diff->to_del][0]);
    *probes_a_curr = _mm256_xor_si256(*probes_a_curr, probes_a_all[comb_diff->to_add][0]);

    *probes_b_curr = _mm256_xor_si256(*probes_b_curr, probes_b_all[comb_diff->to_del][0]);
    *probes_b_curr = _mm256_xor_si256(*probes_b_curr, probes_b_all[comb_diff->to_add][0]);

    *probes_r_curr ^= probes_r_all[comb_diff->to_del][0] ^ probes_r_all[comb_diff->to_add][0];

    return 0;
}

int check_partial(struct comb_t comb_struct, uint64_t nb, int check_sni)
{
    uint64_t nb_internal;
    struct comb_diff_t comb_diff;
    uint64_t probes_r_curr;
    uint64_t *probes_r_all[NB_PR];
    int attack = 0;
    uint64_t c = 0;
    __m256i probes_a_curr;
    __m256i probes_b_curr;
    __m256i *probes_a_all[NB_PR];
    __m256i *probes_b_all[NB_PR];
    init_all(probes_a_all, probes_b_all, probes_r_all);

    init_sh_curr(&probes_a_curr, probes_a_all, comb_struct.combination, comb_struct.k);
    init_sh_curr(&probes_b_curr, probes_b_all, comb_struct.combination, comb_struct.k);
    init_r_curr(&probes_r_curr, probes_r_all, comb_struct.combination, comb_struct.k);

    c = 0;
    nb_internal = 0;
    for (uint64_t i = 0; i < comb_struct.k; i++)
        nb_internal += comb_struct.combination[i] < NB_INT ? 1 : 0;

    while (!attack && c < nb) {
        c++;


        attack = check_support(comb_struct, nb_internal, probes_a_curr,
                probes_b_curr, probes_r_curr, probes_a_all, probes_b_all,
                probes_r_all, check_sni);
        if (next_support(&comb_struct, &comb_diff, &probes_a_curr, &probes_b_curr,
                    &probes_r_curr, probes_a_all, probes_b_all, probes_r_all,
                    &nb_internal, check_sni)) {
            break;
        }
    }

    free_all(probes_a_all, probes_b_all, probes_r_all);
    if (attack) {
        printf("\n");
        print_combination(comb_struct);
        printf("to_del=%lu\n", comb_diff.to_del);
        printf("to_add=%lu\n", comb_diff.to_add);
        printf("\n------- UNSAFE -------\n");
        if (check_sni) {
            printf("ATTACK SNI FOUND\n");
        } else {
            printf("ATTACK NI FOUND\n");
        }
        return 0;
    }

    return 1;
}

#else

int check_partial(struct comb_t comb_struct, uint64_t nb, int check_sni)
{
    uint64_t nb_internal;
    struct comb_diff_t comb_diff;
    uint64_t probes_r_curr[SIZE_R];
    int attack_sni = 0;
    int attack_ni = 0;
    uint64_t c = 0;
    uint64_t probes_a_curr[NB_SH][SIZE_SH];
    uint64_t probes_b_curr[NB_SH][SIZE_SH];

    init_sh_curr(probes_a_curr, probes_sh_a, comb_struct.combination, comb_struct.k);
    init_sh_curr(probes_b_curr, probes_sh_b, comb_struct.combination, comb_struct.k);
    init_r_curr(probes_r_curr, comb_struct.combination, comb_struct.k);

    c = 0;
    nb_internal = 0;
    for (uint64_t i = 0; i < comb_struct.k; i++)
        nb_internal += comb_struct.combination[i] < NB_INT ? 1 : 0;

    while (!attack_ni && !attack_sni && c < nb) {
        c++;

        if (check_sni) {
            attack_sni = check_attack_sni(comb_struct.k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
        } else {
            attack_ni = check_attack_ni(comb_struct.k, probes_r_curr, probes_a_curr, probes_b_curr);
        }
        next_combination(&comb_struct, &comb_diff);
        if (comb_struct.done) break;

        // Adjust the number of internal probes
        if (check_sni) {
            if (comb_diff.to_del < NB_INT) nb_internal--;
            if (comb_diff.to_add < NB_INT) nb_internal++;
        }

        probes_sh_xor(probes_a_curr, probes_sh_a[comb_diff.to_del]);
        probes_sh_xor(probes_a_curr, probes_sh_a[comb_diff.to_add]);

        probes_sh_xor(probes_b_curr, probes_sh_b[comb_diff.to_del]);
        probes_sh_xor(probes_b_curr, probes_sh_b[comb_diff.to_add]);

        probes_r_xor(probes_r_curr, probes_r[comb_diff.to_del]);
        probes_r_xor(probes_r_curr, probes_r[comb_diff.to_add]);
    }

    if (attack_ni || attack_sni) {
        printf("\n");
        print_combination(comb_struct);
        printf("to_del=%lu\n", comb_diff.to_del);
        printf("to_add=%lu\n", comb_diff.to_add);
        printf("\n------- UNSAFE -------\n");
        if (attack_ni) printf("ATTACK NI FOUND\n");
        if (attack_sni) printf("ATTACK SNI FOUND\n");
        return 0;
    }

    return 1;
}
#endif /* VECT */
