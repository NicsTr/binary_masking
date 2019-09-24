#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <immintrin.h>

#include "prob_desc.h"

#ifdef VECT

/* Hamming weight of v as a vector of 16x16 */

#if defined(__AVX512VL__) && defined(__AVX512BW__)
// Probably the best?
int popcount256_16(__m256i v)
{
	return __builtin_popcountl(_mm256_cmpgt_epi16_mask(v, _mm256_setzero_si256()));
}
#elif defined(__AVX2__)
int popcount256_16(__m256i v)
{
	__m256i zero, live, gathv;
	uint8_t gath[32] = {0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
						0x00, 0x02, 0x04, 0x06, 0x08, 0x0A, 0x0C, 0x0E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

	zero  = _mm256_setzero_si256();
	gathv = _mm256_loadu_si256((__m256i*)gath);

	live = _mm256_cmpgt_epi16(v, zero);
	live = _mm256_srli_epi16(live, 15);
	live = _mm256_shuffle_epi8(live, gathv);

	return	  __builtin_popcountl(_mm256_extract_epi64(live,0))
			+ __builtin_popcountl(_mm256_extract_epi64(live,2));
}

#else
#error AVX2 or AVX512 extensions needed
#endif

#endif /* VECT */
