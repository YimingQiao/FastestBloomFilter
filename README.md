# Fastest-Bloom-Filter

Fastest-Bloom-Filter is a high-performance C++ library that implements various Bloom filter designs, each optimized for specific use cases. The library provides a unified interface for all Bloom filter implementations, enabling seamless integration and easy benchmarking.

## Features

- **Multiple Implementations**: Includes a variety of Bloom filter designs optimized for different performance and memory trade-offs.
- **Unified Interface**: All implementations share a common interface for ease of use and interchangeability.
- **High Performance**: Designed for speed and low memory usage.
- **Configurable Parameters**: Supports tuning of false positive rates and memory usage.

## Shared Interface

All Bloom filters in this repository implement the following interface:

- **Constructor**: Initializes the Bloom filter with the number of keys and bits per key.
  ```cpp
  BloomFilterType(size_t num_keys, uint32_t num_bits_per_key);
  ```

- **Insert**: Inserts a batch of hashed keys into the Bloom filter.
  ```cpp
  void Insert(uint32_t num, uint64_t *key);
  ```

- **Lookup**: Checks the membership of a batch of hashed keys and outputs the results.
  ```cpp
  uint32_t Lookup(uint32_t num, uint64_t *key, uint32_t *out);
  ```

This shared interface allows you to easily switch between different Bloom filter implementations without modifying your application logic.

## Build Instructions

To build the project, follow these steps:

1. Clone the repository:

   ```bash
   git clone https://github.com/YimingQiao/Fastest-Bloom-Filter.git
   cd Fastest-Bloom-Filter
   ```

2. Create a build directory and navigate into it:

   ```bash
   mkdir build
   cd build
   ```

3. Configure the project using CMake. To enable or disable AVX-512 optimizations, modify the `USE_AVX512` option directly in the `CMakeLists.txt` file:

   ```cmake
   # In CMakeLists.txt
   option(USE_AVX512 "Enable AVX-512 optimizations" ON) # Set to OFF to disable
   ```

   After modifying the file, run:

   ```bash
   cmake ..
   ```

4. Build the project:

   ```bash
   cmake --build .
   ```

## Usage

After building the project, you can use any of the Bloom filter implementations in your C++ code. Below is an example using the `CacheSectorizedBF32Bit` implementation:

```cpp
#include "cache_sectorized_BF_32bit.h"

#include <cstdint>
#include <iostream>

int main() {
  using bloom_filters::CacheSectorizedBF32Bit;

  // Initialize the Bloom filter with 1000 keys and 10 bits per key.
  CacheSectorizedBF32Bit bloom_filter(/*num_keys=*/1000, /*num_bits_per_key=*/10);

  // Example keys.
  uint64_t keys[] = {0x123456789ABCDEF0, 0x0FEDCBA987654321};
  uint32_t results[2] = {0};

  // Insert keys into the Bloom filter.
  bloom_filter.Insert(/*num=*/2, /*key=*/keys);

  // Lookup keys in the Bloom filter.
  bloom_filter.Lookup(/*num=*/2, /*key=*/keys, /*out=*/results);

  // Check results.
  for (int i = 0; i < 2; ++i) {
    if (results[i]) {
      std::cout << "Key " << i << " is in the set.\n";
    } else {
      std::cout << "Key " << i << " is not in the set.\n";
    }
  }

  return 0;
}
```

## Benchmarking

The repository includes a benchmarking tool (`main_benchmark.cpp`) to evaluate the performance of different Bloom filter implementations. You can run the benchmark manually with the following command:

```bash
./Fastest-Bloom-Filter <num_keys> <num_bits_per_key> <num_lookup_times>
```

For example:

```bash
./Fastest-Bloom-Filter 15 16 26
```

This command benchmarks the Bloom filters with 2<sup>15</sup> keys, 16 bits per key, and 2<sup>26</sup> lookup operations.

### Automated Benchmarking Script

To simplify running benchmarks with multiple parameter combinations, the repository provides an automated script: `scripts/run_benchmarks.py`. This script runs the benchmarks with predefined parameter ranges and saves the results to a specified directory.

#### Usage

1. Ensure the benchmark executable is built and available (e.g., `./build/main_benchmark`).
2. Run the script:

   ```bash
   python3 scripts/run_benchmarks.py --executable ./build/main_benchmark --output-dir benchmark_results
   ```

   - `--executable`: Path to the benchmark executable (default: `./build/main_benchmark`).
   - `--output-dir`: Directory to save the benchmark results (default: `benchmark_results`).

#### Example Output

The script will generate result files in the specified output directory, named according to the parameters used (e.g., `keys_12_bits_32_lookups_24.txt`).

#### Customizing Parameters

You can modify the parameter ranges in the script by editing the following variables:

- `num_keys_powers`: Powers of 2 for the number of keys (e.g., `[12, 14, 16, 18]`).
- `bits_per_key`: Number of bits per key (e.g., `[32]`).
- `lookup_powers`: Powers of 2 for the number of lookups (e.g., `[24]`).

## Contributing

Contributions are welcome! If you would like to contribute, please fork the repository, create a feature branch, and submit a pull request. Ensure your code adheres to the project's coding standards and includes appropriate tests.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.

## Reference

This project is inspired by the techniques described in the following paper:

**Harald Lang, Thomas Neumann, Alfons Kemper, and Peter Boncz**. *Performance-optimal filtering: Bloom overtakes Cuckoo at high throughput*. Proceedings of the VLDB Endowment, Vol. 12, No. 5, 2019, pp. 502–515. [Link to paper](https://doi.org/10.14778/3303753.3303757)


