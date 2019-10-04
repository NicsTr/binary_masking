#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "checker.h"
#include "combinations.h"
#include "prob_desc.h"

struct partial_thread_t {
    struct comb_t *comb_struct;
    uint64_t nb;
    int check_sni;
    int *is_safe;
    pthread_mutex_t *sub_thread_mut;
    pthread_mutex_t *is_safe_mut;
    pthread_barrier_t *barrier;
};

void usage(char *bin_name)
{
    fprintf(stderr, "Usage: %s [--sni] [-p nb_threads] [-n nb_sets_max] [-h, --help]\n", bin_name);
    fprintf(stderr, "\t-p nb_threads: Divide the verification effort among nb_threads threads\n");
    fprintf(stderr, "\t-n nb_sets_max: Don't paralellize until the number of sets to check is greater than 2^nb_sets_max\n");
    fprintf(stderr, "\t-w weight: Only check attack sets of weight weight\n");
    fprintf(stderr, "\t-c chunk_id num_chunks: (for parallel verification only) only check the chunk_id^th (starting from 0) 1/num_chunks fraction of the attack sets\n");
}

void stop_threads(pthread_t *threads, uint32_t nb_thr)
{
    uint32_t i;

    for (i = 0; i < nb_thr; i++) {
        pthread_cancel(threads[i]);
    }
}

void thread_wrapper(struct partial_thread_t *partial_thread)
{
    struct comb_t comb_struct = *(partial_thread->comb_struct);
    uint64_t nb = partial_thread->nb;
    int check_sni = partial_thread->check_sni;
    int *is_safe = partial_thread->is_safe;
    int is_safe_loc;
    pthread_mutex_t *sub_thread_mut = partial_thread->sub_thread_mut;
    pthread_mutex_t *is_safe_mut = partial_thread->is_safe_mut;
    pthread_barrier_t *barrier = partial_thread->barrier;

    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    is_safe_loc = check_partial(comb_struct, nb, check_sni);



    /* Avoid multiple read/write on is_safe */
    pthread_mutex_lock(is_safe_mut);
    *is_safe &= is_safe_loc;
    pthread_mutex_unlock(is_safe_mut);

    /* Ensure that only one "sub-thread" is waiting at the barrier */
    pthread_mutex_lock(sub_thread_mut);
    /* Allow the routine in main to check is_safe */
    pthread_barrier_wait(barrier);
    pthread_mutex_unlock(sub_thread_mut);

    pthread_exit(NULL);
}


int main(int argc, char **argv)
{
    int check_sni = 0;
    uint64_t i = 1;
    uint32_t nb_thr = 1;
    uint64_t nb_sets_max = 25;
    int is_safe = 1;
    uint64_t nb_sets;
    uint64_t chunk_size;
    uint64_t offset;
    struct partial_thread_t **all_partial_thread;
    struct comb_t *comb_struct;
    uint64_t *comb;
	int weight = -1;
	int chunk_id = -1;
	int num_chunk = -1;
    pthread_t *threads;
    pthread_barrier_t barrier;
    pthread_mutex_t sub_thread_mut, is_safe_mut;


    while ((uint64_t) argc > i) {
        if (!strcmp(argv[i], "--sni")) {
            check_sni = 1;
            i++;
        } else if ((uint64_t) argc > i + 1 && !strcmp(argv[i], "-p") && atoi(argv[i + 1]) > 0) {
            nb_thr = atoi(argv[i + 1]);
            i += 2;
        } else if ((uint64_t) argc > i + 1 && !strcmp(argv[i], "-n")) {
            nb_sets_max = atoi(argv[i + 1]);
            i += 2;
        } else if ((uint64_t) argc > i + 1 && !strcmp(argv[i], "-w")) {
            weight = atoi(argv[i + 1]);
            i += 2;
        } else if ((uint64_t) argc > i + 1 && !strcmp(argv[i], "-c")) {
            chunk_id = atoi(argv[i + 1]);
            num_chunk = atoi(argv[i + 2]);
            i += 3;
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return -1;
        } else {
            usage(argv[0]);
            return -1;
        }
    }


    if (nb_thr == 1)
        return check_full(check_sni, weight/* ugly */);


    /* Parallelization */

    printf("Checking security for %s...\n", filename);
    if (check_sni) {
        printf("Stopping at first SNI attack\n");
    } else {
        printf("Stopping at first NI attack\n");
    }

    all_partial_thread = calloc(nb_thr, sizeof(struct partial_thread_t *));
    threads = calloc(nb_thr, sizeof(pthread_t));
    for (int k = (weight == - 1 ? 1 : weight); k <= (weight == -1 ? D : weight); k++) {
        printf("\nSearching for an attack with %d probes:\n", k);
        nb_sets = nCr(NB_PR, k);
        if (nb_sets < (uint64_t) 2 << nb_sets_max) {
            /* Too small to multithread */
            printf("/* Too small to multithread %d */\n", k);
            if (weight != -1)
				printf("/* Checking all %lu sets (unless an attack is found) */\n", nb_sets);
            comb_struct = calloc(1, sizeof(struct comb_t));
            comb = calloc(k + 1, sizeof(uint64_t));
            init_combination(comb_struct, comb, k, NB_PR);
            is_safe &= check_partial(*comb_struct, nb_sets, check_sni);
            free(comb);
            free(comb_struct);
        } else {
			/* Chunkify */
			if (num_chunk != -1)
			{	
				chunk_size = (nb_sets/num_chunk) + 1; // play it stupid safe
				offset = chunk_id*chunk_size;
				printf("/* Checking attack sets of %d probes from #%lu to #%lu out of #%lu */\n", k, offset, (offset+chunk_size - 1) % nb_sets, nb_sets);
			}
			else
			{
				chunk_size = nb_sets;
				offset = 0;
			}
            /* Init pthread struct to control flow */
            pthread_mutex_init(&is_safe_mut, NULL);
            pthread_mutex_init(&sub_thread_mut, NULL);
            pthread_barrier_init(&barrier, NULL, 2);

            /* Create `nb_thr` threads and call the wrapper with the right
             * parameters */
#ifdef BOUNDARY_CHECKS
			uint64_t last_check = 0;
			uint64_t first_check = 0;
#endif
            for (i = 0; i < nb_thr; i++) {
                all_partial_thread[i] = calloc(1, sizeof(struct partial_thread_t));
                comb_struct = calloc(1, sizeof(struct comb_t));
                comb = calloc(k + 1, sizeof(uint64_t));
                init_combination(comb_struct, comb, k, NB_PR);
                unrank(i*chunk_size/nb_thr + offset, *comb_struct);
                all_partial_thread[i]->comb_struct = comb_struct;
                all_partial_thread[i]->nb = (1 + chunk_size/nb_thr);
                all_partial_thread[i]->check_sni = check_sni;
                all_partial_thread[i]->is_safe = &is_safe;
                all_partial_thread[i]->barrier = &barrier;
                all_partial_thread[i]->is_safe_mut = &is_safe_mut;
                all_partial_thread[i]->sub_thread_mut = &sub_thread_mut;

#ifdef BOUNDARY_CHECKS
				first_check = i*chunk_size/nb_thr + offset;
				if (i > 0)
				{
					if (first_check > (last_check + 1))
					   printf("NOOOOOO %lu %lu %lu\n", i, last_check, first_check);
				}
				last_check = i*chunk_size/nb_thr + offset+ chunk_size/nb_thr;
				if (i == nb_thr - 1)
				{
					if (last_check < (offset + chunk_size - 1))
					   printf("NOOOOOO %lu %lu %lu\n", i, last_check, (offset + chunk_size - 1));
				}
#endif

                pthread_create(&threads[i], NULL, (void *) thread_wrapper, all_partial_thread[i]);
            }

            /* When a thread finishes, check for early abort */
            for (i = 0; i < nb_thr; i++) {
                pthread_barrier_wait(&barrier);
                pthread_mutex_lock(&is_safe_mut);
                if (!is_safe)
                    break;
                pthread_mutex_unlock(&is_safe_mut);
            }


            /* If not safe, then cancel all threads */
            for (i = 0; i < nb_thr; i++) {
                if (!is_safe) {
                    printf("\n------- UNSAFE -------\n");
                    pthread_cancel(threads[i]);
                }
                pthread_join(threads[i], NULL);
                free(all_partial_thread[i]->comb_struct->combination);
                free(all_partial_thread[i]->comb_struct);
                free(all_partial_thread[i]);
            }

            pthread_barrier_destroy(&barrier);
            pthread_mutex_destroy(&sub_thread_mut);
            pthread_mutex_destroy(&is_safe_mut);
        }

        if (!is_safe) break;
		if (num_chunk != -1)
		{	
			chunk_size = (nb_sets/num_chunk) + 1; // play it stupid safe
			offset = chunk_id*chunk_size;
			printf("\n/*... checked */\n");
		}
		else
		{
			printf("\rSets of probes already visited: %lu", nb_sets);
		}
        fflush(stdout);
    }

    free(all_partial_thread);
    free(threads);

    if (is_safe) {
		if (weight == -1) 
			printf("\n-------  SAFE  -------\n");
		else
			printf("\n------- No attack found with %d probes -------\n", weight);
    } else {
        printf("\n------- UNSAFE -------\n");
    }

    return is_safe;


}
