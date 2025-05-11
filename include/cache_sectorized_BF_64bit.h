#pragma once

#include "base.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <sys/types.h>
#include <vector>

namespace bloom_filters {

class CacheSectorizedBF64Bit {
public:
	const uint64_t MAX_NUM_BLOCKS = (1ULL << 32);
	static constexpr uint32_t SECTOR_BITS = 64;
	static constexpr uint32_t BLOCK_BITS = 512;
	static constexpr uint32_t SECTORS_PER_BLOCK = BLOCK_BITS / SECTOR_BITS; // 8 sectors per block
	static constexpr uint32_t NUM_BITS = 8;

public:
	explicit CacheSectorizedBF64Bit(size_t n_key, uint32_t n_bits_per_key) {
		size_t raw_blocks = ((n_key * n_bits_per_key + BLOCK_BITS - 1) / BLOCK_BITS);
		size_t exp = static_cast<size_t>(std::ceil(std::log2(raw_blocks))) + 1;
		num_blocks = std::min(1ULL << exp, MAX_NUM_BLOCKS);

		blocks.resize(num_blocks * SECTORS_PER_BLOCK, 0);

		double bits_per_key = static_cast<double>(num_blocks * SECTORS_PER_BLOCK * 64) / static_cast<double>(n_key);
		std::cout << "CSBF Size: " << (num_blocks * SECTORS_PER_BLOCK * sizeof(uint64_t)) / 1024 << " KiB\n";
		std::cout << "Bits per key: " << bits_per_key << "\n";
	}

	inline void Insert(size_t num, uint64_t *BF_RESTRICT key) {
		uint64_t *BF_RESTRICT bf = blocks.data();
		for (size_t i = 0; i < num; ++i) {
			uint64_t full_hash = key[i];
			uint32_t h1 = static_cast<uint32_t>(full_hash);
			uint32_t h2 = static_cast<uint32_t>(full_hash >> 32);

			size_t block = (h2 >> 4) & (num_blocks - 1);
			uint32_t group_a_sector = h1 & 0x3;
			uint32_t group_b_sector = h1 >> 2 & 0x3;
			uint64_t *BF_RESTRICT block_base = bf + block * SECTORS_PER_BLOCK;

			uint64_t mask_a = 0;
			uint64_t mask_b = 0;
			for (int j = 0; j < 4; ++j) {
				mask_a |= 1ULL << ((h1 + j * h2) & 0x3F);
				mask_b |= 1ULL << ((h2 + j * h1) & 0x3F);
			}

			block_base[group_a_sector] |= mask_a;
			block_base[4 + group_b_sector] |= mask_b;
		}
	}

	inline size_t Lookup(size_t num, uint64_t *BF_RESTRICT key, uint32_t *BF_RESTRICT out) const {
		const uint64_t *BF_RESTRICT bf = blocks.data();
		for (size_t i = 0; i < num; ++i) {
			uint64_t full_hash = key[i];
			uint32_t h1 = static_cast<uint32_t>(full_hash);
			uint32_t h2 = static_cast<uint32_t>(full_hash >> 32);

			size_t block = (h2 >> 4) & (num_blocks - 1);
			uint32_t group_a_sector = h2 & 0x3;
			uint32_t group_b_sector = h2 >> 2 & 0x3;
			const uint64_t *BF_RESTRICT block_base = bf + block * SECTORS_PER_BLOCK;

			uint64_t mask_a = 0;
			uint64_t mask_b = 0;
			for (int j = 0; j < 4; ++j) {
				mask_a |= 1ULL << ((h1 + j * h2) & 0x3F);
				mask_b |= 1ULL << ((h2 + j * h1) & 0x3F);
			}

			bool match = ((block_base[group_a_sector] & mask_a) ^ mask_a) == 0;
			match &= ((block_base[4 + group_b_sector] & mask_b) ^ mask_b) == 0;

			out[i] = match;
		}
		return num;
	}

private:
	size_t num_blocks;
	std::vector<uint64_t, AlignedAllocator<uint64_t, 64>> blocks;
};
} // namespace bloom_filters