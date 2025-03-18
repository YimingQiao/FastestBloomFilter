#include "base.h"
#include "register_blocked_BF_32bit.h"
#include "register_blocked_BF_64bit.h"
#include "register_blocked_BF_32bit_Masks.h"
#include "register_blocked_BF_64bit_Masks.h"
#include "cache_sectorized_BF.h"

#include <cstdint>
#include <iostream>
#include <x86intrin.h>

template <typename BloomFilterType, typename HashType>
void RunBenchmark(const std::string &title, size_t num_bits_per_key, size_t num_keys, size_t num_lookup_times) {
	// Create a Bloom filter
	BloomFilterType bf(num_bits_per_key, num_keys);

	// Prepare keys
	std::vector<uint64_t> keys(num_keys);
	for (size_t i = 0; i < num_keys; i++) {
		keys[i] = i;
	}
	std::vector<uint64_t> lookup_keys(num_keys);
	for (size_t i = 0; i < num_keys; i++) {
		lookup_keys[i] = i + num_keys;
	}

	std::vector<HashType> hashes(num_keys);

	// Insert
	uint64_t start = __rdtsc();
	bloom_filters::HashVector(num_keys, keys.data(), hashes.data());
	bf.Insert(num_keys, hashes.data());
	uint64_t end = __rdtsc();
	double insert_cpt = static_cast<double>(end - start) / static_cast<double>(num_keys);

	// Lookup
	const size_t lookupRepeat = std::max(num_lookup_times / num_keys, 1UL);
	std::vector<uint32_t> out(num_keys, 0);
	start = __rdtsc();
	for (size_t r = 0; r < lookupRepeat; r++) {
		bloom_filters::HashVector(num_keys, lookup_keys.data(), hashes.data());
		bf.Lookup(num_keys, hashes.data(), out.data());
	}
	end = __rdtsc();
	double lookup_cpt = static_cast<double>(end - start) / static_cast<double>(num_lookup_times);

	// False-positive rate
	size_t false_positives = 0;
	for (size_t i = 0; i < num_keys; i++) {
		if (out[i]) {
			false_positives++;
		}
	}
	double fp_rate = static_cast<double>(false_positives) / static_cast<double>(num_keys);

	std::cout << "[" << title << "]\n"
	          << "Insert took " << insert_cpt << " cycles per tuple\n"
	          << "Lookup took " << lookup_cpt << " cycles per tuple\n"
	          << "False-positive rate ~ " << fp_rate << "\n\n";
}

void ParseArgs(int argc, char *argv[], size_t &num_keys, size_t &num_bits_per_key, size_t &num_lookup_times) {
	if (argc != 4 && argc != 1) {
		std::cerr << "Usage: " << argv[0] << " <num_keys> <num_bits_per_key> <num_lookup_times>\n";
		exit(1);
	}

	if (argc == 4) {
		num_keys = 1 << std::stoi(argv[1]);
		num_bits_per_key = std::stoi(argv[2]);
		num_lookup_times = 1 << std::stoi(argv[3]);

		if (num_keys <= 0 || num_bits_per_key <= 0 || num_lookup_times <= 0) {
			std::cerr << "All arguments must be positive integers\n";
			exit(1);
		}
	}

	std::cout << "Number of keys: " << num_keys << "\n";
	std::cout << "Number of bits per key: " << num_bits_per_key << "\n";
	std::cout << "Number of lookup times: " << num_lookup_times << "\n\n";
}

int main(int argc, char *argv[]) {
	size_t num_keys = (1 << 14);
	size_t num_bits_per_key = 14;
	size_t num_lookup_times = std::max(1UL << 20, num_keys);

	ParseArgs(argc, argv, num_keys, num_bits_per_key, num_lookup_times);

	RunBenchmark<bloom_filters::RegisterBlockedBF32Bit, uint32_t>(
	    "32-bit Vectorized Register-Blocked BF", num_bits_per_key, num_keys, num_lookup_times);

	RunBenchmark<bloom_filters::RegisterBlockedBF32BitMasks, uint32_t>(
	    "32-bit Vectorized Register-Blocked BF with Masks", num_bits_per_key, num_keys, num_lookup_times);

	RunBenchmark<bloom_filters::RegisterBlockedBF64Bit, uint64_t>(
	    "64-bit Vectorized Register-Blocked BF", num_bits_per_key, num_keys, num_lookup_times);

	RunBenchmark<bloom_filters::RegisterBlockedBF64BitMasks, uint64_t>(
	    "64-bit Vectorized Register-Blocked BF with Masks", num_bits_per_key, num_keys, num_lookup_times);

	// RunBenchmark<bloom_filters::CacheSectorizedBloomFilter, uint64_t>(
	//     "64-bit Vectorized Cache-sectorized BF", num_bits_per_key, num_keys, num_lookup_times);

	return 0;
}
