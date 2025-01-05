#pragma once

#include <algorithm>
#include <stdexcept>
#include <cstdint>
#include <vector>

template <typename T>
class CrossProductIterator {
private:
    // Reference to input vectors
    std::vector<std::vector<T>>& vectors;
    // Current indices for each vector
    std::vector<std::size_t> current_indices;
    // Flag to indicate end of iteration
    bool is_end;

    // Helper method to increment indices
    void increment() {
        if (is_end) return;
        
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
    using pointer = const std::vector<T>*;
    using reference = const std::vector<T>&;



    // Constructor
    // Copy constructor
    CrossProductIterator(const CrossProductIterator& other)
        : vectors(other.vectors)
        , current_indices(other.current_indices)
        , is_end(other.is_end) {}

      CrossProductIterator& operator=(const CrossProductIterator& other) {
        if (this != &other) {
            // Reference stays bound to the same vectors
            current_indices = other.current_indices;
            is_end = other.is_end;
        }
        return *this;
    }

    // Constructor
    explicit CrossProductIterator(std::vector<std::vector<T>>& input_vectors)
        : vectors(input_vectors)
        , current_indices(input_vectors.size(), 0)
        , is_end(false) {
        // Handle empty input case
        if (input_vectors.empty() || std::any_of(input_vectors.begin(), 
                                                input_vectors.end(),
                                                [](const auto& v) { return v.empty(); })) {
            is_end = true;
        }
    }

    // End iterator constructor
    CrossProductIterator(std::vector<std::vector<T>>& input_vectors, bool end)
        : vectors(input_vectors)
        , current_indices(input_vectors.size(), 0)
        , is_end(true) {}

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
    CrossProductIterator& operator++() {
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
    bool operator==(const CrossProductIterator& other) const {
        if (is_end && other.is_end) return true;
        if (is_end || other.is_end) return false;
        return current_indices == other.current_indices;
    }

    bool operator!=(const CrossProductIterator& other) const {
        return !(*this == other);
    }
};

template <typename T>
auto make_cross_product_iterator(std::vector<std::vector<T>>& input) {
    return std::pair{
        CrossProductIterator<T>(input),
        CrossProductIterator<T>(input, true)
    };
}


template<typename T = int>
class MultisetIterator {
private:
    std::vector<T> current;
    size_t n_elements;
    T range_size;
    bool is_end;

public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::vector<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = const std::vector<T>*;
    using reference = const std::vector<T>&;

    // Helper function to compute combinations (n choose k)
    static size_t compute_combination(size_t n, size_t k) {
        if (k > n) return 0;
        if (k == 0 || k == n) return 1;
        if (k > n/2) k = n - k;  // optimize: C(n,k) = C(n,n-k)
        
        size_t result = 1;
        for (size_t i = 0; i < k; ++i) {
            result *= (n - i);
            result /= (i + 1);
        }
        return result;
    }

    // Constructor
    MultisetIterator(size_t n, T s, bool end = false) 
        : current(n), n_elements(n), range_size(s), is_end(end) {
        if (!end) {
            std::fill(current.begin(), current.end(), 0);
        } else {
            std::fill(current.begin(), current.end(), range_size);
        }
    }

    // Calculate total number of possible multisets
    size_t total_size() const {
        return compute_combination(n_elements + range_size - 1, n_elements);
    }

    // Find next valid multiset
    void next_multiset() {
        for (int i = n_elements - 1; i >= 0; --i) {
            if (current[i] + 1 < range_size && (i == 0 || current[i] + 1 <= current[i-1])) {
                ++current[i];
                for (size_t j = i + 1; j < n_elements; ++j) {
                    current[j] = current[i];
                }
                return;
            }
        }
        is_end = true;
        std::fill(current.begin(), current.end(), range_size);
    }

    // Find previous valid multiset
    void prev_multiset() {
        if (is_end) {
            is_end = false;
            std::fill(current.begin(), current.end(), range_size - 1);
            return;
        }

        for (int i = n_elements - 1; i >= 0; --i) {
            if (current[i] > 0 && (i == n_elements - 1 || current[i] > current[i+1])) {
                --current[i];
                for (size_t j = i + 1; j < n_elements; ++j) {
                    current[j] = range_size - 1;
                }
                return;
            }
        }
    }

    // Convert current multiset to position
    difference_type position() const {
        if (is_end) return total_size();

        difference_type pos = 0;
        MultisetIterator it(n_elements, range_size);
        
        while (it != *this) {
            ++pos;
            it.next_multiset();
        }
        
        return pos;
    }

    // Convert position to multiset
    void set_position(difference_type pos) {
        if (pos >= static_cast<difference_type>(total_size())) {
            is_end = true;
            std::fill(current.begin(), current.end(), range_size);
            return;
        }

        is_end = false;
        std::fill(current.begin(), current.end(), 0);
        
        for (difference_type i = 0; i < pos; ++i) {
            next_multiset();
        }
    }

    // Random access operations
    MultisetIterator& operator+=(difference_type n) {
        set_position(position() + n);
        return *this;
    }

    MultisetIterator operator+(difference_type n) const {
        MultisetIterator result = *this;
        result += n;
        return result;
    }

    friend MultisetIterator operator+(difference_type n, const MultisetIterator& it) {
        return it + n;
    }

    MultisetIterator& operator-=(difference_type n) {
        return *this += -n;
    }

    MultisetIterator operator-(difference_type n) const {
        MultisetIterator result = *this;
        result -= n;
        return result;
    }

    difference_type operator-(const MultisetIterator<T>& other) const {
        return position() - other.position();
    }

    // Basic iterator operations
    reference operator*() const { return current; }
    pointer operator->() const { return &current; }

    MultisetIterator& operator++() {
        next_multiset();
        return *this;
    }

    MultisetIterator operator++(int) {
        MultisetIterator tmp = *this;
        next_multiset();
        return tmp;
    }

    MultisetIterator& operator--() {
        prev_multiset();
        return *this;
    }

    MultisetIterator operator--(int) {
        MultisetIterator tmp = *this;
        prev_multiset();
        return tmp;
    }

    // Comparison operators
    bool operator==(const MultisetIterator<T>& other) const {
        if (is_end && other.is_end) return true;
        if (is_end || other.is_end) return false;
        return current == other.current;
    }

    bool operator!=(const MultisetIterator<T>& other) const {
        return !(*this == other);
    }

    bool operator<(const MultisetIterator<T>& other) const {
        if (is_end) return false;
        if (other.is_end) return true;
        return position() < other.position();
    }

    bool operator>(const MultisetIterator<T>& other) const {
        return other < *this;
    }

    bool operator<=(const MultisetIterator<T>& other) const {
        return !(other < *this);
    }

    bool operator>=(const MultisetIterator<T>& other) const {
        return !(*this < other);
    }
};

// Range helper classes
template<typename T = int>
class MultisetRange {
    size_t n_elements;
    T range_size;

public:
    MultisetRange(size_t n, T s) : n_elements(n), range_size(s) {}

    MultisetIterator<T> begin() const {
        return MultisetIterator<T>(n_elements, range_size);
    }

    MultisetIterator<T> end() const {
        return MultisetIterator<T>(n_elements, range_size, true);
    }
};

template<typename T = int>
MultisetRange<T> make_multiset_range(size_t n, T s) {
    return MultisetRange<T>(n, s);
}