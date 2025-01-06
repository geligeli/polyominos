#include "combinatorics.hpp"

uint64_t binomialCoeff(uint64_t n, uint64_t k) {
    if (k > n) return 0;
    if (k == 0 || k == n) return 1;
    if (k > n - k) k = n - k;
    uint64_t result = 1;
    for (uint64_t i = 0; i < k; ++i) {
        result *= n - i;
        result /= i + 1;
    }
    return result;
}

#include <iostream>

std::vector<std::size_t> getCombinationIndices(uint64_t n, uint64_t k, uint64_t index) {
    std::vector<std::size_t> result;
    result.reserve(k);
    for (uint64_t i = 0; i < k; ++i) {
        // start with n=k-1 ... inf and find the last choose(n,k) < index
        uint64_t n_search = k-i-1;
        while(binomialCoeff(n_search+1, k-i) <= index) {
            n_search++;
        }
        index -= binomialCoeff(n_search, k);

        std::cout << "n_search: " << n_search <<  " " << binomialCoeff(n_search, k) << " remainder index: " << index << std::endl;
        // std::cout << "binomialCoeff: " << binomialCoeff(n_search, k-i) << std::endl;


        result.push_back(n_search);
    }
    return result;
}