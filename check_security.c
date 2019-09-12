#include <immintrin.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>

#include "combinations.h"
#include "prob_desc.h"
#include "checker.h"

#ifdef VECT

int main(int argc, char **argv)
{
    uint64_t k;
    uint64_t nb_internal;
    int64_t i;
    uint16_t to_del, to_add;
    uint64_t comb[D + 1];
    uint64_t probes_r_curr;
    int attack_sni = 0;
    int attack_ni = 0;
    int check_sni = 0;
    uint64_t c = 0;
    __m256i probes_a_curr;
    __m256i probes_b_curr;
    __m256i probes_a_all[NB_PR];
    __m256i probes_b_all[NB_PR];
    init_sh_all(probes_a_all, probes_b_all);

    printf("Checking security for %s...\n", filename);
    printf("Stopping at first NI attack\n");
    if (argc > 1 && !strncmp(argv[1], "--sni", 5)) {
        check_sni = 1;
        printf("Stopping at first SNI attack\n");
    }

    for (k = 1; k <= D; k++) {
        printf("\nAttacks with %lu probes:\n", k);
        init_combination(comb, k, NB_PR);
        init_sh_curr(&probes_a_curr, probes_a_all, comb, k);
        init_sh_curr(&probes_b_curr, probes_b_all, comb, k);
        init_r_curr(&probes_r_curr, comb, k);

        c = 0;
        nb_internal = k;
        while (!attack_ni && !attack_sni) {
            c++;
            if (!(c % 100000000)) {
                printf("\rSets of probes already visited: %lu", c);
                fflush(stdout);
            }
            if (check_sni)
                attack_sni = check_attack_sni(k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
            attack_ni = check_attack_ni(k, probes_r_curr, probes_a_curr, probes_b_curr);
            i = next_combination(comb, k);
            if (i == 0) break;
            to_del = (uint16_t) ((i >> 16) & 0xFFFF);
            to_add = (uint16_t) (i & 0xFFFF);

            // Adjust the number of internal probes
            if (to_del < NB_PR - NB_SH) nb_internal--;
            if (to_add < NB_PR - NB_SH) nb_internal++;

            probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[to_del]);
            probes_a_curr = _mm256_xor_si256(probes_a_curr, probes_a_all[to_add]);

            probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[to_del]);
            probes_b_curr = _mm256_xor_si256(probes_b_curr, probes_b_all[to_add]);

            probes_r_curr ^= probes_r[to_del] ^ probes_r[to_add];
        }

        if (attack_ni || attack_sni) {
            printf("\n");
            print_combination(comb, k);
            printf("to_del=%d\n", to_del);
            printf("to_add=%d\n", to_add);
            printf("\n------- UNSAFE -------\n");
            if (attack_ni) printf("ATTACK NI FOUND\n");
            if (attack_sni) printf("ATTACK SNI FOUND\n");
            return 1;
        }
        printf("\rSets of probes already visited: %lu", c);
        fflush(stdout);

    }

    printf("\n-------  SAFE  -------\n");
    return 0;

}
#else

int main(int argc, char **argv)
{
    uint64_t k;
    uint64_t nb_internal;
    int64_t i;
    uint16_t to_del, to_add;
    uint64_t comb[D + 1];
    uint64_t probes_r_curr[SIZE_R];
    int attack_sni = 0;
    int attack_ni = 0;
    int check_sni = 0;
    uint64_t c = 0;
    uint64_t probes_a_curr[NB_SH][SIZE_SH];
    uint64_t probes_b_curr[NB_SH][SIZE_SH];

    printf("Checking security for %s...\n", filename);
    printf("Stopping at first NI attack\n");
    if (argc > 1 && !strncmp(argv[1], "--sni", 5)) {
        check_sni = 1;
        printf("Stopping at first SNI attack\n");
    }

    for (k = 1; k <= D; k++) {
        printf("\nAttacks with %lu probes:\n", k);
        init_combination(comb, k, NB_PR);
        init_sh_curr(probes_a_curr, probes_sh_a, comb, k);
        init_sh_curr(probes_b_curr, probes_sh_b, comb, k);
        init_r_curr(probes_r_curr, comb, k);

        c = 0;
        nb_internal = k;
        while (!attack_ni && !attack_sni) {
            c++;
            if (!(c % 100000000)) {
                printf("\rSets of probes already visited: %lu", c);
                fflush(stdout);
            }
            if (check_sni)
                attack_sni = check_attack_sni(k, nb_internal, probes_r_curr, probes_a_curr, probes_b_curr);
            attack_ni = check_attack_ni(k, probes_r_curr, probes_a_curr, probes_b_curr);
            i = next_combination(comb, k);
            if (i == 0) break;
            to_del = (uint16_t) ((i >> 16) & 0xFFFF);
            to_add = (uint16_t) (i & 0xFFFF);

            // Adjust the number of internal probes
            if (to_del < NB_PR - NB_SH) nb_internal--;
            if (to_add < NB_PR - NB_SH) nb_internal++;

            probes_sh_xor(probes_a_curr, probes_sh_a[to_del]);
            probes_sh_xor(probes_a_curr, probes_sh_a[to_add]);

            probes_sh_xor(probes_b_curr, probes_sh_b[to_del]);
            probes_sh_xor(probes_b_curr, probes_sh_b[to_add]);

            probes_r_xor(probes_r_curr, probes_r[to_del]);
            probes_r_xor(probes_r_curr, probes_r[to_add]);
        }

        if (attack_ni || attack_sni) {
            printf("\n");
            print_combination(comb, k);
            printf("to_del=%d\n", to_del);
            printf("to_add=%d\n", to_add);
            printf("\n------- UNSAFE -------\n");
            if (attack_ni) printf("ATTACK NI FOUND\n");
            if (attack_sni) printf("ATTACK SNI FOUND\n");
            return 1;
        }
        printf("\rSets of probes already visited: %lu", c);
        fflush(stdout);

    }

    printf("\n-------  SAFE  -------\n");
    return 0;

}
#endif /* VECT */
