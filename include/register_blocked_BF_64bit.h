#pragma once

#include "base.h"

#include <cmath>
#include <cstddef>
#include <cstring>
#include <cstdint>
#include <iostream>
#include <vector>

namespace bloom_filters {
class RegisterBlockedBF64Bit {
public:
	const uint32_t MAX_NUM_BLOCKS = (1 << 31);

public:
	explicit RegisterBlockedBF64Bit(size_t n_key, uint32_t n_bits_per_key) {
		num_blocks = ((n_key * n_bits_per_key) >> 6) + 1;
		num_blocks_log = static_cast<uint32_t>(std::log2(num_blocks)) + 1;
		num_blocks = std::min(1U << num_blocks_log, MAX_NUM_BLOCKS);

		blocks.resize(num_blocks);
		std::cout << "BF Size: " << num_blocks * 8 / 1024 << " KiB\n";
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
			uint32_t block = (key[i] >> (64 - num_blocks_log)) & (num_blocks - 1);
			uint64_t mask = (1 << (key[i] & 63)) | (1 << ((key[i] >> 6) & 63)) | (1 << ((key[i] >> 12) & 63)) |
			                (1 << ((key[i] >> 18) & 63));
			bf[block] |= mask;
		}
	}
	size_t LookupInternal(size_t num, uint64_t *BF_RESTRICT key, uint64_t *BF_RESTRICT bf,
	                      uint32_t *BF_RESTRICT out) const {
		for (size_t i = 0; i < num; i++) {
			uint32_t block = (key[i] >> (64 - num_blocks_log)) & (num_blocks - 1);
			uint64_t mask = (1 << (key[i] & 63)) | (1 << ((key[i] >> 6) & 63)) | (1 << ((key[i] >> 12) & 63)) |
			                (1 << ((key[i] >> 18) & 63));
			out[i] = (bf[block] & mask) == mask;

			// If we add a if here, we will get the correct false-postive rate for the 64-bit BF
			// if (i == 9){
			// 	std::cout << "key: " << key[i] << " block: " << block << " mask: " << mask << " bf[block]: " <<
			// bf[block] << " out: " << out[i] << "\n";
			// }
		}
		return num;
	}

private:
	size_t num_blocks;
	size_t num_blocks_log;
	std::vector<uint64_t> blocks;
};
} // namespace bloom_filters