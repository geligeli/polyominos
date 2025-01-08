#pragma once

#include <algorithm>
#include <compare>
#include <cstdint>
#include <stdexcept>
#include <sys/types.h>
#include <type_traits>
#include <vector>

uint64_t binomialCoeff(uint64_t n, uint64_t k);
uint64_t multichoose(uint64_t n, uint64_t k);
std::vector<std::size_t> getCombinationIndices(uint64_t k, uint64_t index);
void DecreaseKSubsets(std::vector<uint64_t> &indices);
void IncreaseKSubsets(std::vector<uint64_t> &indices);

class SubSetsGenerator {
public:
  // Iterator traits
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::vector<uint64_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<uint64_t> *;
  using reference = const std::vector<uint64_t> &;
  // Constructor
  SubSetsGenerator(uint64_t n, uint64_t k, uint64_t index);
  // Dereference
  reference operator*() const;
  pointer operator->() const;

  // Increment/Decrement
  SubSetsGenerator &operator++();
  SubSetsGenerator operator++(int);
  SubSetsGenerator &operator--();
  SubSetsGenerator operator--(int);

  // Arithmetic operations
  SubSetsGenerator &operator+=(difference_type diff);
  SubSetsGenerator operator+(difference_type diff) const;
  SubSetsGenerator &operator-=(difference_type diff);
  SubSetsGenerator operator-(difference_type diff) const;
  difference_type operator-(const SubSetsGenerator &other) const;
  std::strong_ordering operator<=>(const SubSetsGenerator &other) const;
  bool operator==(const SubSetsGenerator &other) const;
  value_type operator[](difference_type diff) const;

private:
  uint64_t n;
  uint64_t index;
  std::vector<uint64_t> storage;
};

// Helper class to create begin/end iterators
class SubSetsRange {
private:
  uint64_t n;
  uint64_t k;
  uint64_t size;

public:
  SubSetsRange(uint64_t n, uint64_t k);
  uint64_t num_dims() const;
  uint64_t num_subsets() const;

  SubSetsGenerator begin() const;
  SubSetsGenerator end() const;
  bool operator==(const SubSetsRange &other) const;
};


// class SubSetsRangeProduct {
// public:
//   using iterator_category = std::random_access_iterator_tag;
//   using value_type = std::vector<uint64_t>;
//   using difference_type = std::ptrdiff_t;
//   using pointer = const std::vector<uint64_t> *;
//   using reference = const std::vector<uint64_t> &;

//   struct eof_iterator {};

//   SubSetsRangeProduct(std::vector<SubSetsRange> sequences, eof_iterator);
//   explicit SubSetsRangeProduct(std::vector<SubSetsRange> sequences);

//   // Dereference
//   reference operator*();
//   pointer operator->();

//   // Increment/Decrement
//   SubSetsRangeProduct &operator++();
//   SubSetsRangeProduct operator++(int);
//   SubSetsRangeProduct &operator--();
//   SubSetsRangeProduct operator--(int);

//   // Arithmetic operations
//   SubSetsRangeProduct &operator+=(difference_type diff);
//   SubSetsRangeProduct operator+(difference_type diff) const;
//   SubSetsRangeProduct &operator-=(difference_type diff);
//   SubSetsRangeProduct operator-(difference_type diff) const;
//   difference_type operator-(const SubSetsRangeProduct &other) const;

//   // Comparison operators
//   std::strong_ordering operator<=>(const SubSetsRangeProduct &other) const;
//   bool operator==(const SubSetsRangeProduct &other) const;
//   value_type operator[](difference_type diff) const;

// private:
//   uint64_t m_index;
//   std::vector<SubSetsRange> m_sequences;
//   std::vector<SubSetsGenerator> m_indices;
//   std::vector<uint64_t> m_n;
//   std::vector<uint64_t> m_storage;

//   void update_storage();
//   void inc();
//   void dec();
//   void sync_index();
// };

// Helper class to create begin/end iterators


std::vector<std::size_t> getMultiSetCombinationIndices(uint64_t k,
                                                       uint64_t index);
void DecreaseKMultiSet(std::vector<uint64_t> &indices);
void IncreaseKMultiSet(std::vector<uint64_t> &indices);

class MultiSetsGenerator {
public:
  // Iterator traits
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::vector<uint64_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<uint64_t> *;
  using reference = const std::vector<uint64_t> &;
  // Constructor
  MultiSetsGenerator(uint64_t n, uint64_t k, uint64_t index);
  // Dereference
  reference operator*() const;
  pointer operator->() const;

  // Increment/Decrement
  MultiSetsGenerator &operator++();
  MultiSetsGenerator operator++(int);
  MultiSetsGenerator &operator--();
  MultiSetsGenerator operator--(int);

  // Arithmetic operations
  MultiSetsGenerator &operator+=(difference_type diff);
  MultiSetsGenerator operator+(difference_type diff) const;
  MultiSetsGenerator &operator-=(difference_type diff);
  MultiSetsGenerator operator-(difference_type diff) const;
  difference_type operator-(const MultiSetsGenerator &other) const;
  std::strong_ordering operator<=>(const MultiSetsGenerator &other) const;
  bool operator==(const MultiSetsGenerator &other) const;
  value_type operator[](difference_type diff) const;

private:
  uint64_t m_n;
  uint64_t m_index;
  std::vector<uint64_t> m_storage;
};

// Helper class to create begin/end iterators
class MultiSetsRange {
private:
  uint64_t n;
  uint64_t k;
  uint64_t size;

public:
  MultiSetsRange(uint64_t n, uint64_t k);
  uint64_t num_dims() const;
  uint64_t num_subsets() const;

  MultiSetsGenerator begin() const;
  MultiSetsGenerator end() const;
  bool operator==(const MultiSetsRange &other) const;
};

template <class T> class RangeProduct {
public:
  using iterator_category = std::random_access_iterator_tag;
  using value_type = std::vector<uint64_t>;
  using difference_type = std::ptrdiff_t;
  using pointer = const std::vector<uint64_t> *;
  using reference = const std::vector<uint64_t> &;

  struct eof_iterator {};

  RangeProduct(std::vector<T> sequences, eof_iterator)
      : m_index(1), m_sequences(std::move(sequences)) {
    uint64_t num_dimensions{};
    for (const auto &seq : m_sequences) {
      m_indices.push_back(seq.end());
      num_dimensions += seq.num_dims();
      m_index *= seq.num_subsets();
    }
    m_storage.resize(num_dimensions);
  }

  explicit RangeProduct(std::vector<T> sequences)
      : m_index(0), m_sequences(std::move(sequences)) {
    uint64_t num_dimensions{};
    for (const auto &seq : m_sequences) {
      m_indices.push_back(seq.begin());
      num_dimensions += seq.num_dims();
    }
    m_storage.resize(num_dimensions);
    update_storage();
  }

  // Dereference
  reference operator*() {
    update_storage();
    return m_storage;
  }
  pointer operator->() {
    update_storage();
    return &m_storage;
  }

  // Increment/Decrement
  RangeProduct &operator++() {
    inc();
    return *this;
  }

  RangeProduct operator++(int) {
    RangeProduct tmp = *this;
    inc();
    return tmp;
  }
  RangeProduct &operator--() {
    dec();
    return *this;
  }
  RangeProduct operator--(int) {
    RangeProduct tmp = *this;
    dec();
    return tmp;
  }
  // Arithmetic operations
  RangeProduct &operator+=(difference_type diff) {
    m_index += diff;
    sync_index();
    return *this;
  }
  RangeProduct operator+(difference_type diff) const {
    auto result = *this;
    return result += diff;
  }
  RangeProduct &operator-=(difference_type diff) {
    m_index -= diff;
    sync_index();
    return *this;
  }
  RangeProduct operator-(difference_type diff) const {
    auto result = *this;
    return result -= diff;
  }
  difference_type operator-(const RangeProduct &other) const {
    return m_index - other.m_index;
  }

  // Comparison operators
  std::strong_ordering operator<=>(const RangeProduct &other) const {
    return m_index <=> other.m_index;
  }
  bool operator==(const RangeProduct &other) const {
    return m_index == other.m_index && m_sequences == other.m_sequences;
  }
  value_type operator[](difference_type diff) const {
    auto result = *this;
    result += diff;
    return *std::move(result);
  }

private:
  uint64_t m_index;
  std::vector<T> m_sequences;
  std::vector<std::decay_t<decltype(std::declval<T>().begin())>> m_indices;
  std::vector<uint64_t> m_storage;

  void update_storage() {
    auto insert_pos = m_storage.begin();
    for (const auto &seq : m_indices) {
      insert_pos = std::copy(seq->begin(), seq->end(), insert_pos);
    }
  }
  void inc() {
    ++m_index;
    for (int i = 0; i < m_sequences.size(); ++i) {
      if (++m_indices[i] != m_sequences[i].end()) {
        return;
      }
      m_indices[i] = m_sequences[i].begin();
    }
  }
  void dec() {
    --m_index;
    for (int i = 0; i < m_sequences.size(); ++i) {
      if (m_indices[i] != m_sequences[i].begin()) {
        --m_indices[i];
        return;
      }
      m_indices[i] = std::prev(m_sequences[i].end());
    }
  }

  void sync_index() {
    auto work_idx = m_index;
    for (uint64_t i = 0; i < m_sequences.size(); ++i) {
      m_indices[i] =
          m_sequences[i].begin() + (work_idx % m_sequences[i].num_subsets());
      work_idx /= m_sequences[i].num_subsets();
    }
  }
};

template <class T> class RangeProductRange {
public:
  RangeProductRange(std::vector<T> sequences) : sequences(std::move(sequences)) {}
  RangeProduct<T> begin() const {
    return RangeProduct<T>(sequences);
  }
  RangeProduct<T> end() const {
    return RangeProduct<T>(sequences, typename RangeProduct<T>::eof_iterator());
  }

private:
  std::vector<T> sequences;
};

using SubSetsRangeProduct = RangeProduct<SubSetsRange>;
using SubSetsRangeProductRange = RangeProductRange<SubSetsRange>;

using MultiSetsRangeProduct = RangeProduct<MultiSetsRange>;
using MultiSetsRangeProductRange = RangeProductRange<MultiSetsRange>;
