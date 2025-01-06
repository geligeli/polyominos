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

uint64_t binomialCoeff(uint64_t n, uint64_t k);

std::vector<std::size_t> getCombinationIndices(uint64_t n, uint64_t k, uint64_t index);