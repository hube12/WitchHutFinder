#ifndef JAVARND_H_
#define JAVARND_H_

#include <stdint.h>


/********************** C copy of the Java Random methods **********************
 */

static inline void setSeed(int64_t *seed) {
	*seed = (uint64_t) ((uint64_t) *seed ^ 0x5deece66du) & ((1LLu << 48u) - 1);
}

static inline int next(int64_t *seed, const int bits) {
	*seed = ((uint64_t)*seed * 0x5deece66du + 0xbu) & ((1LLu << 48u) - 1);
	return (int) ((uint64_t)*seed >> (48u - bits));
}

static inline int nextInt(int64_t *seed, const int n) {
	int bits, val;
	const int m = n - 1;

	if (((uint64_t)m & (uint64_t)n) == 0) return (int) ((uint64_t)(n * (long) next(seed, 31)) >> 31u);

	do {
		bits = next(seed, 31);
		val = bits % n;
	} while (bits - val + m < 0);
	return val;
}

static inline int64_t nextLong(int64_t *seed) {
	return ((uint64_t)(int64_t) next(seed, 32) << 32u) + next(seed, 32);
}

static inline double nextDouble(int64_t *seed) {
	return (double)(((uint64_t) next(seed, 26) << 27u) + next(seed, 27)) / (double) (1LLu << 53u);
}

#endif /* JAVARND_H_ */
