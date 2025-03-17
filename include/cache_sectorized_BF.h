#pragma once

#include "base.h"

#include <cstddef> // for size_t
#include <iostream>
#include <vector>
#include <cstdint>

namespace bloom_filters {
class CacheSectorizedBloomFilter {
public:
	const uint64_t sector_size = 64;
	const uint64_t num_groups_per_block = 2;
	const uint64_t num_groups_per_block_log = 1;
	const uint64_t block_size = 512;

	const uint64_t num_sector_per_group = 4;
	const uint64_t num_sector_per_group_log = 2;

public:
	explicit CacheSectorizedBloomFilter(size_t n_key, uint32_t n_bits_per_key) {
		num_blocks = ((n_key * n_bits_per_key) >> 9) + 1;
		num_blocks_log = static_cast<uint64_t>(std::log2(num_blocks)) + 1;
		num_blocks = std::min(1UL << num_blocks_log, 1UL << 16);

		blocks.resize(num_blocks * num_groups_per_block * num_sector_per_group, 0);
		std::cout << "BF Size: " << num_blocks * block_size / 64 / 1024 << " KiB\n";
	}

public:
	inline void Insert(size_t num, uint64_t *key) {
		InsertInternal(num, key, blocks.data());
	}

	inline size_t Lookup(size_t num, uint64_t *key, uint32_t *out) {
		return LookupInternal(num, key, blocks.data(), out);
	}

public:
	void InsertInternal(size_t num, uint64_t *BF_RESTRICT key, uint64_t *BF_RESTRICT bf) const {
		for (size_t i = 0; i < num; i++) {
			uint64_t block = ((key[i] >> (64 - num_blocks_log)) & (num_blocks - 1))
			                 << num_groups_per_block_log << num_sector_per_group_log;
			for (size_t j = 0; j < num_groups_per_block; j++) {
				uint32_t sector =
				    (key[i] >> (64 - num_blocks_log - num_sector_per_group_log - j)) & (num_sector_per_group - 1);
				uint64_t mask = (1 << ((key[i] >> (j * 24)) & 63)) | (1 << ((key[i] >> (j * 24 + 4)) & 63)) |
				                (1 << ((key[i] >> (j * 24 + 12)) & 63)) | (1 << ((key[i] >> (j * 24 + 18)) & 63));
				bf[block + j * num_sector_per_group + sector] |= mask;
			}
		}
		return;
	}
	size_t LookupInternal(size_t num, uint64_t *BF_RESTRICT key, uint64_t *BF_RESTRICT bf,
	                      uint32_t *BF_RESTRICT out) const {
		for (size_t i = 0; i < num; i++) {
			uint64_t block = ((key[i] >> (64 - num_blocks_log)) & (num_blocks - 1))
			                 << num_groups_per_block_log << num_sector_per_group_log;
			out[i] = 1;
			for (size_t j = 0; j < num_groups_per_block; j++) {
				uint32_t sector =
				    (key[i] >> (64 - num_blocks_log - num_sector_per_group_log - j)) & (num_sector_per_group - 1);
				uint64_t mask = (1 << ((key[i] >> (j * 24)) & 63)) | (1 << ((key[i] >> (j * 24 + 4)) & 63)) |
				                (1 << ((key[i] >> (j * 24 + 12)) & 63)) | (1 << ((key[i] >> (j * 24 + 18)) & 63));
				out[i] &= (bf[block + j * num_sector_per_group + sector] & mask) == mask;
			}
		}
		return num;
	}

private:
	size_t num_blocks;
	size_t num_blocks_log;
	std::vector<uint64_t> blocks;
};
} // namespace bloom_filters