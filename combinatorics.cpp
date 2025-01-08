#include "combinatorics.hpp"
#include <cstdint>

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

uint64_t multichoose(uint64_t n, uint64_t k) {
  return binomialCoeff(n + k - 1, k);
}

std::vector<std::size_t> getCombinationIndices(uint64_t k, uint64_t index) {
  std::vector<std::size_t> result(k, 0);
  for (uint64_t i = 0; i < k; ++i) {
    // start with n=k-1 ... inf and find the last choose(n,k) < index
    uint64_t n_search = k - i - 1;
    while (binomialCoeff(n_search + 1, k - i) <= index) {
      n_search++;
    }
    result[i] = n_search;
    const uint64_t delta = binomialCoeff(n_search, k - i);
    if (delta > index) {
      break;
    }
    index -= delta;
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

// Constructor
SubSetsGenerator::SubSetsGenerator(uint64_t n, uint64_t k, uint64_t index)
    : n(n), index(index), storage(getCombinationIndices(k, index)) {}

// Dereference
SubSetsGenerator::reference SubSetsGenerator::operator*() const {
  return storage;
}
SubSetsGenerator::pointer SubSetsGenerator::operator->() const {
  return &storage;
}

// Increment/Decrement
SubSetsGenerator &SubSetsGenerator::operator++() {
  ++index;
  IncreaseKSubsets(storage);
  return *this;
}
SubSetsGenerator SubSetsGenerator::operator++(int) {
  SubSetsGenerator tmp = *this;
  ++index;
  IncreaseKSubsets(storage);
  return tmp;
}
SubSetsGenerator &SubSetsGenerator::operator--() {
  --index;
  DecreaseKSubsets(storage);
  return *this;
}
SubSetsGenerator SubSetsGenerator::operator--(int) {
  SubSetsGenerator tmp = *this;
  --index;
  DecreaseKSubsets(storage);
  return tmp;
}

// Arithmetic operations
SubSetsGenerator &
SubSetsGenerator::operator+=(SubSetsGenerator::difference_type diff) {
  index += diff;
  storage = getCombinationIndices(storage.size(), index);
  return *this;
}
SubSetsGenerator
SubSetsGenerator::operator+(SubSetsGenerator::difference_type diff) const {
  return SubSetsGenerator(n, storage.size(), index + diff);
}
SubSetsGenerator &
SubSetsGenerator::operator-=(SubSetsGenerator::difference_type diff) {
  index -= diff;
  storage = getCombinationIndices(storage.size(), index);
  return *this;
}
SubSetsGenerator
SubSetsGenerator::operator-(SubSetsGenerator::difference_type diff) const {
  return SubSetsGenerator(n, storage.size(), index - diff);
}
SubSetsGenerator::difference_type
SubSetsGenerator::operator-(const SubSetsGenerator &other) const {
  return index - other.index;
}

// Comparison operators
std::strong_ordering
SubSetsGenerator::operator<=>(const SubSetsGenerator &other) const {
  return index <=> other.index;
}

bool SubSetsGenerator::operator==(const SubSetsGenerator &other) const {
  return n == other.n && storage.size() == other.storage.size() &&
         index == other.index;
}

// Array subscript
SubSetsGenerator::value_type
SubSetsGenerator::operator[](difference_type diff) const {
  return getCombinationIndices(storage.size(), index + diff);
}

bool SubSetsRange::operator==(const SubSetsRange &other) const {
  return n == other.n && k == other.k;
}

SubSetsRange::SubSetsRange(uint64_t n, uint64_t k)
    : n(n), k(k), size(binomialCoeff(n, k)) {}

uint64_t SubSetsRange::num_dims() const { return k; }
uint64_t SubSetsRange::num_subsets() const { return size; }

SubSetsGenerator SubSetsRange::begin() const {
  return SubSetsGenerator(n, k, 0);
}
SubSetsGenerator SubSetsRange::end() const {
  return SubSetsGenerator(n, k, size);
}

std::vector<std::size_t> getMultiSetCombinationIndices(uint64_t k,
                                                       uint64_t index) {
  std::vector<std::size_t> result(k, 0);
  for (uint64_t i = 0; i < k; ++i) {
    // start with n=k-1 ... inf and find the last choose(n,k) < index
    uint64_t n_search = 0;
    while (multichoose(n_search + 1, k - i) <= index) {
      n_search++;
    }
    result[i] = n_search;
    const uint64_t delta = multichoose(n_search, k - i);
    if (delta > index) {
      break;
    }
    index -= delta;
  }
  return result;
}

void DecreaseKMultiSet(std::vector<uint64_t> &indices) {
  int k = indices.size();
  if (indices[k - 1] > 0) {
    --indices[k - 1];
    return;
  }
  for (int i = 0; i < k - 1; ++i) {
    if (indices[k - i - 1] > indices[k - i]) {
      indices[k - i - 1]--;
      for (int j = k - i; j < k; ++j) {
        indices[j] = indices[j - 1];
      }
      return;
    }
  }
  --indices[0];
  for (int i = 1; i < k; ++i) {
    indices[i] = indices[i - 1];
  }
}

void IncreaseKMultiSet(std::vector<uint64_t> &indices) {
  int k = indices.size();
  for (int i = 1; i < k; ++i) {
    if (indices[k - i] < indices[k - i - 1]) {
      indices[k - i]++;
      for (int j = 0; j < i - 1; ++j) {
        indices[k - j - 1] = 0;
      }
      return;
    }
  }
  ++indices[0];
  for (int i = 0; i < k - 1; ++i) {
    indices[k - i - 1] = 0;
  }
}

MultiSetsGenerator::MultiSetsGenerator(uint64_t n, uint64_t k, uint64_t index)
    : m_n(n), m_index(index), m_storage(getMultiSetCombinationIndices(k, index)) {}

// // Dereference
MultiSetsGenerator::reference MultiSetsGenerator::operator*() const {
  return m_storage;
}

MultiSetsGenerator::pointer MultiSetsGenerator::operator->() const {
  return &m_storage;
}

// // Increment/Decrement
MultiSetsGenerator &MultiSetsGenerator::operator++() {
  ++m_index;
  IncreaseKMultiSet(m_storage);
  return *this;
}

MultiSetsGenerator MultiSetsGenerator::operator++(int) {
  MultiSetsGenerator tmp = *this;
  ++m_index;
  IncreaseKMultiSet(m_storage);
  return tmp;
}
MultiSetsGenerator &MultiSetsGenerator::operator--() {
  --m_index;
  DecreaseKMultiSet(m_storage);
  return *this;
}
MultiSetsGenerator MultiSetsGenerator::operator--(int) {
  MultiSetsGenerator tmp = *this;
  --m_index;
  DecreaseKMultiSet(m_storage);
  return tmp;
}

// // Arithmetic operations
MultiSetsGenerator &
MultiSetsGenerator::operator+=(MultiSetsGenerator::difference_type diff) {
  m_index += diff;
  m_storage = getMultiSetCombinationIndices(m_storage.size(), m_index);
  return *this;
}
MultiSetsGenerator
MultiSetsGenerator::operator+(MultiSetsGenerator::difference_type diff) const {
  return MultiSetsGenerator(m_n, m_storage.size(), m_index + diff);
};
MultiSetsGenerator &
MultiSetsGenerator::operator-=(MultiSetsGenerator::difference_type diff) {
  m_index -= diff;
  m_storage = getMultiSetCombinationIndices(m_storage.size(), m_index);
  return *this;
}
MultiSetsGenerator
MultiSetsGenerator::operator-(MultiSetsGenerator::difference_type diff) const {
  return MultiSetsGenerator(m_n, m_storage.size(), m_index - diff);
};
MultiSetsGenerator::difference_type
MultiSetsGenerator::operator-(const MultiSetsGenerator &other) const {
  return m_index - other.m_index;
};
std::strong_ordering
MultiSetsGenerator::operator<=>(const MultiSetsGenerator &other) const {
  return m_index <=> other.m_index;
};
bool MultiSetsGenerator::operator==(const MultiSetsGenerator &other) const {
  return m_n == other.m_n && m_storage.size() == other.m_storage.size() &&
         m_index == other.m_index;
};
MultiSetsGenerator::value_type
MultiSetsGenerator::operator[](MultiSetsGenerator::difference_type diff) const {
  return getMultiSetCombinationIndices(m_storage.size(), m_index + diff);
};

MultiSetsRange::MultiSetsRange(uint64_t n, uint64_t k) : n(n), k(k) {
  size = multichoose(n, k);
}

uint64_t MultiSetsRange::num_dims() const { return k; }
uint64_t MultiSetsRange::num_subsets() const { return size; }

MultiSetsGenerator MultiSetsRange::begin() const {
  return MultiSetsGenerator(n, k, 0);
}
MultiSetsGenerator MultiSetsRange::end() const {
  return MultiSetsGenerator(n, k, size);
}
bool MultiSetsRange::operator==(const MultiSetsRange &other) const {
  return n == other.n && k == other.k;
}