#include <immintrin.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "combinations.h"
#include "prob_desc.h"
#include "checker_helper.h"
#include "checker.h"

#ifdef VECT

int check_full(int check_sni, int weight)
{
    uint64_t nb_internal;
    uint64_t comb[D + 1];
    struct comb_t comb_struct;
    struct comb_diff_t comb_diff;
    uint64_t probes_r_curr;
    int attack_sni = 0;
    int attack_ni = 0;
    uint64_t c = 0;
    __m256i probes_a_curr;
    __m256i probes_b_curr;
    __m256i probes_a_all[NB_PR];
    __m256i probes_b_all[NB_PR];
    init_sh_all(probes_a_all, probes_b_all);

    printf("Checking security for %s...\n", filename);
    if (check_sni) {
        printf("Stopping at first SNI attack\n");
    } else {
        printf("Stopping at first NI attack\n");
    }

    for (int k = (weight == - 1 ? 1 : weight); k <= (weight == -1 ? D : weight); k++) {
        printf("\nAttacks with %d probes:\n", k);
        init_combination(&comb_struct, comb, k, NB_PR);
        init_sh_curr(&probes_a_curr, probes_a_all, comb, k);
        init_sh_curr(&probes_b_curr, probes_b_all, comb, k);
        init_r_curr(&probes_r_curr, comb, k);

        c = 0;
        nb_internal = k  > (NB_PR - NB_SH) ? (NB_PR - NB_SH) : k; // all starting probes are internal, except in the very odd case where there's so few of them
        while (!attack_ni && !attack_sni) {
            c++;
            if (!(c % 100000000)) {
                printf("\rSets of probes already visited: %lu", c);
                fflush(stdout);
            }
            if (check_sni) {
                attack_sni = check_attack_sni(k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
            } else {
                attack_ni = check_attack_ni(k, probes_r_curr, probes_a_curr, probes_b_curr);
            }
            next_combination(&comb_struct, &comb_diff);
            if (comb_struct.done) break;

            // Adjust the number of internal probes
            if (check_sni) {
                if (comb_diff.to_del < NB_PR - NB_SH) nb_internal--;
                if (comb_diff.to_add < NB_PR - NB_SH) nb_internal++;
            }

            probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[comb_diff.to_del]);
            probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[comb_diff.to_add]);

            probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[comb_diff.to_del]);
            probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[comb_diff.to_add]);

            probes_r_curr ^= probes_r[comb_diff.to_del] ^ probes_r[comb_diff.to_add];
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
        printf("\rSets of probes already visited: %lu", c);
        fflush(stdout);

    }

	if (weight == -1) 
		printf("\n-------  SAFE  -------\n");
	else
		printf("\n------- No attack found with %d probes -------\n", weight);
    
	return 1;

}

int check_partial(struct comb_t comb_struct, uint64_t nb, int check_sni)
{
    uint64_t nb_internal;
    struct comb_diff_t comb_diff;
    uint64_t probes_r_curr;
    int attack_sni = 0;
    int attack_ni = 0;
    uint64_t c = 0;
    __m256i probes_a_curr;
    __m256i probes_b_curr;
    __m256i probes_a_all[NB_PR];
    __m256i probes_b_all[NB_PR];
    init_sh_all(probes_a_all, probes_b_all);

    init_sh_curr(&probes_a_curr, probes_a_all, comb_struct.combination, comb_struct.k);
    init_sh_curr(&probes_b_curr, probes_b_all, comb_struct.combination, comb_struct.k);
    init_r_curr(&probes_r_curr, comb_struct.combination, comb_struct.k);

    c = 0;
	nb_internal = 0;
	for (uint64_t i = 0; i < comb_struct.k; i++)
		nb_internal += comb_struct.combination[i] < (NB_PR - NB_SH) ? 1 : 0;

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
            if (comb_diff.to_del < NB_PR - NB_SH) nb_internal--;
            if (comb_diff.to_add < NB_PR - NB_SH) nb_internal++;
        }

        probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[comb_diff.to_del]);
        probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[comb_diff.to_add]);

        probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[comb_diff.to_del]);
        probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[comb_diff.to_add]);

        probes_r_curr ^= probes_r[comb_diff.to_del] ^ probes_r[comb_diff.to_add];
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

#else

int check_full(int check_sni, int weight)
{
    uint64_t nb_internal;
    uint64_t comb[D + 1];
    struct comb_t comb_struct;
    struct comb_diff_t comb_diff;
    uint64_t probes_r_curr[SIZE_R];
    int attack_sni = 0;
    int attack_ni = 0;
    uint64_t c = 0;
    uint64_t probes_a_curr[NB_SH][SIZE_SH];
    uint64_t probes_b_curr[NB_SH][SIZE_SH];

    printf("Checking security for %s...\n", filename);
    if (check_sni) {
        printf("Stopping at first SNI attack\n");
    } else {
        printf("Stopping at first NI attack\n");
    }

    for (int k = (weight == - 1 ? 1 : weight); k <= (weight == -1 ? D : weight); k++) {
        printf("\nAttacks with %d probes:\n", k);
        init_combination(&comb_struct, comb, k, NB_PR);
        init_sh_curr(probes_a_curr, probes_sh_a, comb, k);
        init_sh_curr(probes_b_curr, probes_sh_b, comb, k);
        init_r_curr(probes_r_curr, comb, k);

        c = 0;
        nb_internal = k  > (NB_PR - NB_SH) ? (NB_PR - NB_SH) : k;
        while (!attack_ni && !attack_sni) {
            c++;
            if (!(c % 100000000)) {
                printf("\rSets of probes already visited: %lu", c);
                fflush(stdout);
            }
            if (check_sni) {
                attack_sni = check_attack_sni(k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
            } else {
                attack_ni = check_attack_ni(k, probes_r_curr, probes_a_curr, probes_b_curr);
            }
            next_combination(&comb_struct, &comb_diff);
            if (comb_struct.done) break;

            // Adjust the number of internal probes
            if (check_sni) {
                if (comb_diff.to_del < NB_PR - NB_SH) nb_internal--;
                if (comb_diff.to_add < NB_PR - NB_SH) nb_internal++;
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
            return 1;
        }
        printf("\rSets of probes already visited: %lu", c);
        fflush(stdout);

    }

	if (weight == -1) 
		printf("\n-------  SAFE  -------\n");
	else
		printf("\n------- No attack found with %d probes -------\n", weight);
    return 0;

}

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
		nb_internal += comb_struct.combination[i] < (NB_PR - NB_SH) ? 1 : 0;

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
            if (comb_diff.to_del < NB_PR - NB_SH) nb_internal--;
            if (comb_diff.to_add < NB_PR - NB_SH) nb_internal++;
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
