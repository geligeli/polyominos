#include "combinatorics.hpp"

uint64_t binomialCoeff(uint64_t n, uint64_t k) {
  if (k > n)
    return 0;
  if (k == 0 || k == n)
    return 1;
  if (k > n - k)
    k = n - k;
  uint64_t result = 1;
  for (uint64_t i = 0; i < k; ++i) {
    result *= n - i;
    result /= i + 1;
  }
  return result;
}

std::vector<std::size_t> getCombinationIndices(uint64_t n, uint64_t k,
                                               uint64_t index) {
  std::vector<std::size_t> result;
  result.reserve(k);
  for (uint64_t i = 0; i < k; ++i) {
    // start with n=k-1 ... inf and find the last choose(n,k) < index
    uint64_t n_search = k - i - 1;
    while (binomialCoeff(n_search + 1, k - i) <= index) {
      n_search++;
    }
    index -= binomialCoeff(n_search, k - i);
    result.push_back(n_search);
  }
  return result;
}

void IncreaseKSubsets(std::vector<uint64_t> &subset) {
  int k = subset.size();
  for (int i = 1; i < k; ++i) {
    if (subset[k - i] + 1 < subset[k - i - 1]) {
      subset[k - i]++;
      for (int j = 0; j < i - 1; ++j) {
        subset[k - j - 1] = j;
      }
      return;
    }
  }
  ++subset[0];
  for (int i = 0; i < k - 1; ++i) {
    subset[k - i - 1] = i;
  }
}

void DecreaseKSubsets(std::vector<uint64_t> &indices) {
  int k = indices.size();
  if (indices[k - 1] > 0) {
    --indices[k - 1];
    return;
  }
  for (int i = 0; i < k - 1; ++i) {
    if (indices[k - i - 1] > indices[k - i] + 1) {
      indices[k - i - 1]--;
      for (int j = k - i; j < k; ++j) {
        indices[j] = indices[j - 1] - 1;
      }
      return;
    }
  }
  --indices[0];
  for (int i = 1; i < k; ++i) {
    indices[i] = indices[i - 1] - 1;
  }
}
