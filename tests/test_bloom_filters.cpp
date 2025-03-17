#include <cassert>
#include <iostream>
#include "bloom_filters.h"

int main()
{
    RegisterBlockedBloomFilter rbf(1024, 3);
    rbf.insert(42);
    assert(rbf.contains(42) && "RegisterBlockedBloomFilter failed to find inserted value");

    CacheSectorizedBloomFilter csbf(1024, 3);
    csbf.insert(42);
    assert(csbf.contains(42) && "CacheSectorizedBloomFilter failed to find inserted value");

    // Basic false-positive check (not guaranteed to pass 100%, but should be rare for small sets).
    bool mightContain = rbf.contains(999999);
    std::cout << "RegisterBlockedBloomFilter contains(999999): " << mightContain << "\n";
    return 0;
}
