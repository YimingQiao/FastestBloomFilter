#include "base.h"
#include "register_blocked_BF_32bit.h"
#include "register_blocked_BF_64bit.h"
#include "register_blocked_BF_32bit_Masks.h"
#include "register_blocked_BF_64bit_Masks.h"

#include <cstdint>
#include <iostream>

template <typename BloomFilterType, typename HashType>
void RunBenchmark(const std::string &title, size_t num_bits_per_key, size_t num_keys, size_t num_lookup_times,
                  const std::vector<uint64_t> &keys, const std::vector<uint64_t> &lookup_keys) {
	BloomFilterType bf(num_bits_per_key, num_keys);

	std::vector<HashType> hashes(num_keys);

	// Insert
	uint64_t start = __rdtsc();
	bloom_filters::HashVector(num_keys, keys.data(), hashes.data());
	bf.Insert(num_keys, hashes.data());
	uint64_t end = __rdtsc();
	double insert_cpt = static_cast<double>(end - start) / static_cast<double>(num_keys);

	// Lookup
	const size_t lookupRepeat = num_lookup_times / num_keys;
	std::vector<uint32_t> out(num_keys);
	start = __rdtsc();
	for (size_t r = 0; r < lookupRepeat; r++) {
		bloom_filters::HashVector(num_keys, lookup_keys.data(), hashes.data());
		bf.Lookup(num_keys, hashes.data(), out.data());
	}
	end = __rdtsc();
	double lookup_cpt = static_cast<double>(end - start) / static_cast<double>(num_lookup_times);

	// False-positive rate
	size_t falsePositives = 0;
	for (size_t i = 0; i < num_keys; i++) {
		if (out[i]) {
			falsePositives++;
		}
	}
	double fp_rate = static_cast<double>(falsePositives) / static_cast<double>(num_keys);

	std::cout << "[" << title << "]\n"
	          << "Insert took " << insert_cpt << " cycles per tuple\n"
	          << "Lookup took " << lookup_cpt << " cycles per tuple\n"
	          << "False-positive rate ~ " << fp_rate << "\n\n";
}

int main() {
	const size_t num_keys = (1 << 12);
	const size_t num_bits_per_key = 12;
	const size_t num_lookup_times = 1 << 20;

	std::cout << "Number of keys: " << num_keys << "\n\n";

	// Prepare keys
	std::vector<uint64_t> keys(num_keys);
	for (size_t i = 0; i < num_keys; i++) {
		keys[i] = static_cast<int>(i);
	}
	std::vector<uint64_t> lookup_keys(num_keys);
	for (size_t i = 0; i < num_keys; i++) {
		lookup_keys[i] = static_cast<int>(i + num_keys);
	}

	RunBenchmark<bloom_filters::RegisterBlockedBF32Bit, uint32_t>(
	    "32-bit Vectorized Register-Blocked BF", num_bits_per_key, num_keys, num_lookup_times, keys, lookup_keys);

	RunBenchmark<bloom_filters::RegisterBlockedBF32BitMasks, uint32_t>(
	    "32-bit Vectorized Register-Blocked BF with Masks", num_bits_per_key, num_keys, num_lookup_times, keys,
	    lookup_keys);

	RunBenchmark<bloom_filters::RegisterBlockedBF64Bit, uint64_t>(
	    "64-bit Vectorized Register-Blocked BF", num_bits_per_key, num_keys, num_lookup_times, keys, lookup_keys);

	RunBenchmark<bloom_filters::RegisterBlockedBF64BitMasks, uint64_t>(
	    "64-bit Vectorized Register-Blocked BF with Masks", num_bits_per_key, num_keys, num_lookup_times, keys,
	    lookup_keys);

	return 0;
}
