#pragma once

#include <algorithm>
#include <cstdint>
#include <stdexcept>
#include <sys/types.h>
#include <vector>

template <typename T> class CrossProductIterator {
private:
  // Reference to input vectors
  std::vector<std::vector<T>> &vectors;
  // Current indices for each vector
  std::vector<std::size_t> current_indices;
  // Flag to indicate end of iteration
  bool is_end;

  // Helper method to increment indices
  void increment() {
    if (is_end)
      return;

    // Start from rightmost index
    int i = current_indices.size() - 1;

    // Increment indices with carry-over
    while (i >= 0) {
      current_indices[i]++;
      if (current_indices[i] < vectors[i].size()) {
        break;
      }
      current_indices[i] = 0;
      i--;
    }

    // If we've wrapped around completely, we're at the end
    if (i < 0) {
      is_end = true;
    }
  }

public:
  // Iterator traits
  using iterator_category = std::forward_iterator_tag;
  using value_type = std::vector<T>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<T> *;
  using reference = const std::vector<T> &;

  // Constructor
  // Copy constructor
  CrossProductIterator(const CrossProductIterator &other)
      : vectors(other.vectors), current_indices(other.current_indices),
        is_end(other.is_end) {}

  CrossProductIterator &operator=(const CrossProductIterator &other) {
    if (this != &other) {
      // Reference stays bound to the same vectors
      current_indices = other.current_indices;
      is_end = other.is_end;
    }
    return *this;
  }

  // Constructor
  explicit CrossProductIterator(std::vector<std::vector<T>> &input_vectors)
      : vectors(input_vectors), current_indices(input_vectors.size(), 0),
        is_end(false) {
    // Handle empty input case
    if (input_vectors.empty() ||
        std::any_of(input_vectors.begin(), input_vectors.end(),
                    [](const auto &v) { return v.empty(); })) {
      is_end = true;
    }
  }

  // End iterator constructor
  CrossProductIterator(std::vector<std::vector<T>> &input_vectors, bool end)
      : vectors(input_vectors), current_indices(input_vectors.size(), 0),
        is_end(true) {}

  // Dereference operator
  std::vector<T> operator*() const {
    if (is_end) {
      throw std::out_of_range("Dereferencing end iterator");
    }
    std::vector<T> result;
    result.reserve(vectors.size());
    for (std::size_t i = 0; i < vectors.size(); ++i) {
      result.push_back(vectors[i][current_indices[i]]);
    }
    return result;
  }

  // Prefix increment
  CrossProductIterator &operator++() {
    increment();
    return *this;
  }

  // Postfix increment
  CrossProductIterator operator++(int) {
    CrossProductIterator tmp = *this;
    increment();
    return tmp;
  }

  // Equality operators
  bool operator==(const CrossProductIterator &other) const {
    if (is_end && other.is_end)
      return true;
    if (is_end || other.is_end)
      return false;
    return current_indices == other.current_indices;
  }

  bool operator!=(const CrossProductIterator &other) const {
    return !(*this == other);
  }
};

template <typename T>
auto make_cross_product_iterator(std::vector<std::vector<T>> &input) {
  return std::pair{CrossProductIterator<T>(input),
                   CrossProductIterator<T>(input, true)};
}

uint64_t binomialCoeff(uint64_t n, uint64_t k);
std::vector<std::size_t> getCombinationIndices(uint64_t n, uint64_t k,
                                               uint64_t index);

void DecreaseKSubsets(std::vector<uint64_t> &indices);
void IncreaseKSubsets(std::vector<uint64_t> &indices);

class SubSetsGenerator {
private:
  uint64_t n;
  uint64_t index;
  std::vector<uint64_t> storage;

public:
  // Iterator traits
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::vector<uint64_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<uint64_t> *;
  using reference = const std::vector<uint64_t> &;

  // Constructor
  SubSetsGenerator(uint64_t n, uint64_t k, uint64_t index)
      : n(n), index(index), storage(getCombinationIndices(n, k, index)) {}

  // Dereference
  reference operator*() const { return storage; }

  // Increment/Decrement
  SubSetsGenerator &operator++() {
    ++index;
    IncreaseKSubsets(storage);
    return *this;
  }
  SubSetsGenerator operator++(int) {
    SubSetsGenerator tmp = *this;
    ++index;
    IncreaseKSubsets(storage);
    return tmp;
  }
  SubSetsGenerator &operator--() {
    --index;
    DecreaseKSubsets(storage);
    return *this;
  }
  SubSetsGenerator operator--(int) {
    SubSetsGenerator tmp = *this;
    --index;
    DecreaseKSubsets(storage);
    return tmp;
  }

  // Arithmetic operations
  SubSetsGenerator &operator+=(difference_type diff) {
    index += diff;
    storage = getCombinationIndices(n, storage.size(), index);
    return *this;
  }
  SubSetsGenerator operator+(difference_type diff) const {
    return SubSetsGenerator(n, storage.size(), index + diff);
  }
  SubSetsGenerator &operator-=(difference_type diff) {
    index -= diff;
    storage = getCombinationIndices(n, storage.size(), index);
    return *this;
  }
  SubSetsGenerator operator-(difference_type diff) const {
    return SubSetsGenerator(n, storage.size(), index - diff);
  }
  difference_type operator-(const SubSetsGenerator &other) const {
    return index - other.index;
  }

  // Comparison operators
  bool operator==(const SubSetsGenerator &other) const {
    return index == other.index;
  }
  bool operator!=(const SubSetsGenerator &other) const {
    return index != other.index;
  }
  bool operator<(const SubSetsGenerator &other) const {
    return index < other.index;
  }
  bool operator>(const SubSetsGenerator &other) const {
    return index > other.index;
  }
  bool operator<=(const SubSetsGenerator &other) const {
    return index <= other.index;
  }
  bool operator>=(const SubSetsGenerator &other) const {
    return index >= other.index;
  }

  // Array subscript
  value_type operator[](difference_type diff) const {
    return getCombinationIndices(n, storage.size(), index + diff);
  }
};

// Helper class to create begin/end iterators
class SubSetsSequence {
private:
  uint64_t n;
  uint64_t k;

public:
  SubSetsSequence(uint64_t n, uint64_t k) : n(n), k(k) {}

  SubSetsGenerator begin() const { return SubSetsGenerator(n, k, 0); }
  SubSetsGenerator end() const { return SubSetsGenerator(n, k, binomialCoeff(n, k)); }
};

class SubSetsSequenceCrossProduct {
private:
  uint64_t index;
  std::vector<SubSetsSequence> sequences;
  std::vector<SubSetsGenerator> indices;
  std::vector<uint64_t> n;
  std::vector<uint64_t> storage;
public:
  explicit SubSetsSequenceCrossProduct(std::vector<SubSetsSequence> sequences)
      : index(0), sequences(std::move(sequences)) {
    uint64_t num_dimensions{};
    for (const auto &seq : sequences) {
        indices.push_back(seq.begin());
        n.push_back(std::distance(seq.begin(), seq.end()));
        num_dimensions+=(*seq.begin()).size();
    }
    storage.resize(num_dimensions);
  }
  
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::vector<uint64_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<uint64_t> *;
  using reference = const std::vector<uint64_t> &;

  // Dereference
  reference operator*() const {
    return storage;
  }

  // Increment/Decrement
  SubSetsSequenceCrossProduct &operator++() {
    ++index;
    for (int i = 0; i < sequences.size(); ++i) {  
        if (++indices[i] == sequences[i].end()) {
            indices[i] = sequences[i].begin();
        } else {
            return *this;
        }
    }
    return *this;
  }

  SubSetsSequenceCrossProduct operator++(int) {
    SubSetsSequenceCrossProduct tmp = *this;
    ++index;
    for (int i = 0; i < sequences.size(); ++i) {  
        if (++indices[i] == sequences[i].end()) {
            indices[i] = sequences[i].begin();
        } else {
            return *this;
        }
    }
    return tmp;
  }

  SubSetsSequenceCrossProduct &operator--() {
    --index;
    for (int i = 0; i < sequences.size(); ++i) {  
        if (indices[i] == sequences[i].begin()) {
            indices[i] = std::prev(sequences[i].end());
        } else {
            --indices[i];
            return *this;
        }
    }
    return *this;
  }

  SubSetsSequenceCrossProduct operator--(int) {
    SubSetsSequenceCrossProduct tmp = *this;
    --index;
    for (int i = 0; i < sequences.size(); ++i) {  
        if (indices[i] == sequences[i].begin()) {
            indices[i] = std::prev(sequences[i].end());
        } else {
            --indices[i];
            return *this;
        }
    }
    return tmp;
  }

  // Arithmetic operations
  SubSetsSequenceCrossProduct &operator+=(difference_type diff) {
    index += diff;
    auto work_idx = index;
    for (uint64_t i = 0; i < sequences.size(); ++i) {
        indices[i] = sequences[i].begin() + (work_idx % n[i]);
        work_idx /= n[i];
    }
    return *this;
  }
  SubSetsSequenceCrossProduct operator+(difference_type diff) const {
    auto result = *this;
    return result += diff;
  }

  SubSetsSequenceCrossProduct &operator-=(difference_type diff) {
    index -= diff;
    auto work_idx = index;
    for (uint64_t i = 0; i < sequences.size(); ++i) {
        indices[i] = sequences[i].begin() + (work_idx % n[i]);
        work_idx /= n[i];
    }
    return *this;
  }
  SubSetsSequenceCrossProduct operator-(difference_type diff) const {
    auto result = *this;
    return result -= diff;
  }
  difference_type operator-(const SubSetsSequenceCrossProduct &other) const {
    return index - other.index;
  }

  // Comparison operators
  bool operator==(const SubSetsSequenceCrossProduct &other) const {
    return index == other.index;
  }
  bool operator!=(const SubSetsSequenceCrossProduct &other) const {
    return index != other.index;
  }
  bool operator<(const SubSetsSequenceCrossProduct &other) const {
    return index < other.index;
  }
  bool operator>(const SubSetsSequenceCrossProduct &other) const {
    return index > other.index;
  }
  bool operator<=(const SubSetsSequenceCrossProduct &other) const {
    return index <= other.index;
  }
  bool operator>=(const SubSetsSequenceCrossProduct &other) const {
    return index >= other.index;
  }

  // Array subscript
  value_type operator[](difference_type diff) const {

    auto result = *this;

    auto work_idx = index + diff;
    for (uint64_t i = 0; i < sequences.size(); ++i) {
        result.indices[i] = sequences[i].begin() + (work_idx % n[i]);
        work_idx /= n[i];
    }

    return *result;
  }
};