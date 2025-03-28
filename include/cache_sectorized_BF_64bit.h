#pragma once

#include "base.h"

#include <cstddef> // for size_t
#include <iostream>
#include <vector>
#include <cstdint>

namespace bloom_filters {

// Each sector has 64 bits, and each block has 8 sectors. We use k = 8 hash functions.
// We use 12 bits to find the block. For each group in the block, we use 2 bits to find the sector (2x2 in total).
// Then, 8 * 6 bits are used to find the bit in the sector.
class CacheSectorizedBF64Bit {
public:
	const uint32_t SECTOR_SIZE = 64;
	const uint32_t GROUP_SIZE = 256;
	const uint32_t BLOCK_SIZE = 512;

	const uint32_t SECTOR_SIZE_LOG = 6;
	const uint32_t GROUP_SIZE_LOG = 8;
	const uint32_t BLOCK_SIZE_LOG = 9;

	const uint64_t MAX_NUM_BLOCKS = (1 << 12);

	// Number of groups in a block
	const uint32_t z = 2;
	// Number of sectors in a group
	const uint32_t s = 4;
	const uint32_t s_log = 2;

public:
	explicit CacheSectorizedBF64Bit(size_t n_key, uint32_t n_bits_per_key) {
		num_blocks = ((n_key * n_bits_per_key) >> BLOCK_SIZE_LOG) + 1;
		num_blocks_log = static_cast<uint64_t>(std::log2(num_blocks)) + 1;
		num_blocks = std::min(1UL << num_blocks_log, MAX_NUM_BLOCKS);
		num_sectors = num_blocks * 8;

		blocks = static_cast<uint64_t *>(std::aligned_alloc(64, static_cast<size_t>(num_sectors) * sizeof(uint64_t)));
		if (!blocks) {
			throw std::bad_alloc();
		}
		std::fill(blocks, blocks + static_cast<size_t>(num_sectors), 0);
		std::cout << "BF Size: " << num_sectors * 8 / 1024 << " KiB\n";
	}

public:
	inline void Insert(int num, uint64_t *key) {
		InsertInternal(num, key, blocks);
	}

	inline size_t Lookup(int num, uint64_t *key, uint32_t *out) {
		return LookupInternal(num, key, blocks, out);
	}

public:
	void InsertInternal(int num, uint64_t *BF_RESTRICT key, uint64_t *BF_RESTRICT bf) const {
#pragma clang loop vectorize_width(16)
		for (size_t i = 0; i < num; i++) {
			uint32_t sector_1 = (((key[i] >> 49) & (num_sectors - 1)) & (~7)) | ((key[i] >> 50) & 3);
			uint64_t mask_1 = (1 << ((key[i]) & 63)) | (1 << ((key[i] >> 6) & 63)) | (1 << ((key[i] >> 12) & 63)) |
			                  (1 << ((key[i] >> 18) & 63));
			bf[sector_1] |= mask_1;

			uint32_t sector_2 = (((key[i] >> 49) & (num_sectors - 1)) & (~7)) | ((key[i] >> 48) & 3) | 4;
			uint64_t mask_2 = (1 << ((key[i] >> 24) & 63)) | (1 << ((key[i] >> 30) & 63)) |
			                  (1 << ((key[i] >> 36) & 63)) | (1 << ((key[i] >> 42) & 63));
			bf[sector_2] |= mask_2;
		}
		return;
	}

	// | Blocks Bits (12 bits) | Sectors Bits (2 bits) | Sectors Bits (2 bits) | Hash Bits (48 bits) |
	int LookupInternal(int num, uint64_t *BF_RESTRICT key, uint64_t *BF_RESTRICT bf, uint32_t *BF_RESTRICT out) const {
#pragma clang loop vectorize_width(16)
		for (int i = 0; i < num; i++) {
			uint32_t sector_1 = (((key[i] >> 49) & (num_sectors - 1)) & (~7)) | ((key[i] >> 50) & 3);
			uint64_t mask_1 = (1 << ((key[i]) & 63)) | (1 << ((key[i] >> 6) & 63)) | (1 << ((key[i] >> 12) & 63)) |
			                  (1 << ((key[i] >> 18) & 63));
			bool match1 = (bf[sector_1] & mask_1) == mask_1;

			uint32_t sector_2 = (((key[i] >> 49) & (num_sectors - 1)) & (~7)) | ((key[i] >> 48) & 3) | 4;
			uint64_t mask_2 = (1 << ((key[i] >> 24) & 63)) | (1 << ((key[i] >> 30) & 63)) |
			                  (1 << ((key[i] >> 36) & 63)) | (1 << ((key[i] >> 42) & 63));
			bool match2 = (bf[sector_2] & mask_2) == mask_2;
			out[i] = match1 && match2;
		}
		return num;
	}

private:
	size_t num_sectors;
	size_t num_blocks;
	size_t num_blocks_log;
	uint64_t *blocks;
};
} // namespace bloom_filters