#pragma once

#include "base.h"

#include <cstddef> // for size_t
#include <iostream>
#include <vector>
#include <cstdint>
#include <cstdlib>

namespace bloom_filters {

// Each sector has 64 bits, and each block has 8 sectors. We use k = 8 hash functions.
// We use 12 bits to find the block. For each group in the block, we use 2 bits to find the sector (2x2 in total).
// Then, 8 * 6 bits are used to find the bit in the sector.
class CacheSectorizedBF32Bit {
public:
	const uint32_t SECTOR_SIZE = 32;
	const uint32_t GROUP_SIZE = 256;
	const uint32_t BLOCK_SIZE = 512;

	const uint32_t SECTOR_SIZE_LOG = 5;
	const uint32_t GROUP_SIZE_LOG = 8;
	const uint32_t BLOCK_SIZE_LOG = 9;

	const uint32_t MAX_NUM_BLOCKS = (1 << 12);

	// Number of groups in a block
	const uint32_t z = 2;
	// Number of sectors in a group
	const uint32_t s = 8;
	const uint32_t s_log = 3;

public:
	explicit CacheSectorizedBF32Bit(size_t n_key, uint32_t n_bits_per_key) {
		num_blocks = ((n_key * n_bits_per_key) >> BLOCK_SIZE_LOG) + 1;
		num_blocks_log = static_cast<uint32_t>(std::log2(num_blocks)) + 1;
		num_blocks = std::min(1U << num_blocks_log, MAX_NUM_BLOCKS);
		num_sectors = num_blocks * 16;

		blocks =
		    static_cast<uint32_t *>(std::aligned_alloc(64, static_cast<size_t>(num_blocks * 16) * sizeof(uint32_t)));
		if (!blocks) {
			throw std::bad_alloc();
		}
		std::fill(blocks, blocks + static_cast<size_t>(num_blocks * 16), 0);
		std::cout << "BF Size: " << num_blocks * 16 * 4 / 1024 << " KiB\n";
	}

public:
	inline void Insert(int num, uint64_t *key) {
		InsertInternal(num, key, blocks);
	}

	inline size_t Lookup(int num, uint64_t *key, uint32_t *out) {
		return LookupInternal(num, key, blocks, out);
	}

public:
	void InsertInternal(int num, uint64_t *BF_RESTRICT key, uint32_t *BF_RESTRICT bf) const {
		for (size_t i = 0; i < num; i++) {
			uint32_t sector_1 = (((key[i] >> 46) & (num_blocks - 1)) << 4) | ((key[i] >> 43) & 7);
			uint32_t mask_1 = (1 << ((key[i]) & 31)) | (1 << ((key[i] >> 5) & 31)) | (1 << ((key[i] >> 10) & 31)) |
			                  (1 << ((key[i] >> 15) & 31));
			bf[sector_1] |= mask_1;

			uint32_t sector_2 = (((key[i] >> 46) & (num_blocks - 1)) << 4) | ((key[i] >> 40) & 7) | 8;
			uint32_t mask_2 = (1 << ((key[i] >> 20) & 31)) | (1 << ((key[i] >> 25) & 31)) |
			                  (1 << ((key[i] >> 30) & 31)) | (1 << ((key[i] >> 35) & 31));
			bf[sector_2] |= mask_2;
		}
		return;
	}

	int LookupInternal(int num, uint64_t *BF_RESTRICT key, uint32_t *BF_RESTRICT bf, uint32_t *BF_RESTRICT out) const {
		for (int i = 0; i < num; i++) {
			uint32_t sector_1 = (((key[i] >> 46) & (num_blocks - 1)) << 4) | ((key[i] >> 43) & 7);
			uint32_t mask_1 = (1 << ((key[i]) & 31)) | (1 << ((key[i] >> 5) & 31)) | (1 << ((key[i] >> 10) & 31)) |
			                  (1 << ((key[i] >> 15) & 31));
			bool match_0 = (bf[sector_1] & mask_1) == mask_1;

			uint32_t sector_2 = (((key[i] >> 46) & (num_blocks - 1)) << 4) | ((key[i] >> 40) & 7) | 8;
			uint32_t mask_2 = (1 << ((key[i] >> 20) & 31)) | (1 << ((key[i] >> 25) & 31)) |
			                  (1 << ((key[i] >> 30) & 31)) | (1 << ((key[i] >> 35) & 31));
			bool match1 = (bf[sector_2] & mask_2) == mask_2;
			out[i] = match_0 && match1;
		}
		return num;
	}

private:
	uint32_t num_sectors;
	uint32_t num_blocks;
	uint32_t num_blocks_log;
	uint32_t *blocks;
};
} // namespace bloom_filters