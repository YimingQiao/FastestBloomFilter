#pragma once

#include "base.h"

#include <cstddef> // for size_t
#include <vector>
#include <cstdint>

namespace bloom_filters {
class CacheSectorizedBloomFilter {
public:
	CacheSectorizedBloomFilter(size_t size, size_t hash_count) : m_size(size), m_hash_count(hash_count), m_bits(size) {
	}

	void Insert(uint64_t value) {
		for (size_t i = 0; i < m_hash_count; ++i) {
			m_bits[Hash(value) % m_size] = 1;
		}
	}

	bool Lookup(uint64_t value) const {
		for (size_t i = 0; i < m_hash_count; ++i) {
			if (m_bits[Hash(value) % m_size] == 0) {
				return false;
			}
		}
		return true;
	}

private:
	size_t m_size;
	size_t m_hash_count;
	std::vector<uint64_t> m_bits;

	size_t Hash(uint64_t value) const {
		return hash_fn(value);
	}
};
} // namespace bloom_filters