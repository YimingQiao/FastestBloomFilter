#pragma once

#ifndef BF_RESTRICT
#if defined(_MSC_VER)
#define BF_RESTRICT __restrict
#elif defined(__GNUC__) || defined(__clang__)
#define BF_RESTRICT __restrict__
#else
#define BF_RESTRICT
#endif
#endif

#include <cstdint>
#include <cstddef>
#include <cmath>
#include <cstring>
#include <cstdint>

namespace bloom_filters {
// I use the same hash fucntion as in DuckDB.
inline uint64_t MurmurHash64(uint64_t x) {
	x ^= x >> 32;
	x *= 0xd6e8feb86659fd93U;
	x ^= x >> 32;
	x *= 0xd6e8feb86659fd93U;
	x ^= x >> 32;
	return x;
}

inline uint32_t MurmurHash32(uint32_t x) {
	x ^= x >> 16;
	x *= 0xd6e8feb9U;
	x ^= x >> 16;
	x *= 0xd6e8feb9U;
	x ^= x >> 16;
	return x;
}

inline void HashVector(size_t num, const uint64_t *key, uint64_t *hashes) {
	for (size_t i = 0; i < num; i++) {
		hashes[i] = MurmurHash64(key[i]);
	}
}

inline void HashVector(size_t num, const uint64_t *key, uint32_t *hashes) {
	for (size_t i = 0; i < num; i++) {
		hashes[i] = MurmurHash32(key[i]);
	}
}
} // namespace bloom_filters