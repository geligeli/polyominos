#pragma once
#include "avx_match.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstring>
#include <iostream>
#include <optional>
#include <thread>
#include <vector>

constexpr std::size_t kMaxPolyominoSize = 9;
extern const std::array<std::vector<CandidateMatchBitmask>, kMaxPolyominoSize>
    kPrecomputedPolyminosMatchSet;
extern const std::array<std::vector<std::vector<std::pair<int8_t, int8_t>>>, kMaxPolyominoSize>
    kPrecomputedPolyominosTypeErased;
extern const std::array<std::string, 14> kColors;

struct SolutionStats {
  uint64_t possibilities_at_last_step{1};
  uint64_t solution_count{0};

  int64_t last_step_difficulty() const;
  std::weak_ordering operator<=>(const SolutionStats &other) const;
};

struct PolyominoIndex {
  std::size_t N;
  std::size_t index;
  inline constexpr std::strong_ordering
  operator<=>(const PolyominoIndex &other) const noexcept = default;
};

// Can be used in PuzzleParams::operator[] to get the masks for a given
// polyomino
struct PolyominoSubsetIndex {
  std::size_t N;
  std::size_t index;
  inline constexpr std::strong_ordering
  operator<=>(const PolyominoSubsetIndex &other) const noexcept = default;
};

using BitMaskType = uint64_t;

struct PuzzleParams {
  template <std::size_t N>
  explicit PuzzleParams(const Polyomino<N> &board) noexcept : N(N) {
    BoardMatcher matcher = PolyominoToBoardMatcher(board);
    // avx512 matcher
    for (std::size_t i = 0; i < kMaxPolyominoSize; ++i) {
      for (std::size_t j = 0; j < kPrecomputedPolyminosMatchSet[i].size();
           ++j) {
        auto result =
            find_matches_avx(matcher, kPrecomputedPolyminosMatchSet[i][j]);
        std::vector<BitMaskType> result_masks(result.begin(), result.end());
        if (result_masks.size() > 0) {
          PolyominoIndex idx{i + 1, j};
          PuzzleParams::Tile tile{idx, std::move(result_masks)};
          possible_tiles_per_size[i].push_back(std::move(tile));
        }
      }
    }   
  }

  struct Tile {
    PolyominoIndex polyomino_index;
    std::vector<BitMaskType> masks;
  };

  uint64_t possibilities_for_partition(const std::vector<int> &partition) const;
  double possibilities_for_configuration(
      const std::vector<PolyominoSubsetIndex> &configuration) const;

  const std::vector<BitMaskType> &
  operator[](PolyominoSubsetIndex idx) const noexcept;
  const std::vector<std::pair<int8_t, int8_t>> &xy_coordinates (PolyominoSubsetIndex idx) const noexcept;

  std::size_t N;
  std::array<std::vector<Tile>, kPrecomputedPolyminosMatchSet.size()>
      possible_tiles_per_size;
};

class PuzzleSolver {
public:
  enum class Algoritm { DLX, BF };
  PuzzleSolver(const PuzzleParams &params);

  bool Solve(const std::vector<PolyominoSubsetIndex> &candidate_tiles,
             std::vector<std::size_t> &foundSolution,
             Algoritm algo = Algoritm::BF) const noexcept;

  double
  EstimateDifficulty(const std::vector<PolyominoSubsetIndex> &candidate_tiles,
                     Algoritm algo = Algoritm::BF) const noexcept;

  template <std::size_t N>
  std::string decodeSolution(
      Polyomino<N> board, const std::vector<std::size_t> &solution,
      const std::vector<PolyominoSubsetIndex> &candidate_tiles) const noexcept;

private:
  bool internalSolve(const std::vector<PolyominoSubsetIndex> &candidate_tiles,
                     std::vector<std::size_t> &indices,
                     BitMaskType current_state = 0,
                     std::size_t current_index = 0) const noexcept;
  uint64_t internalCountSolutionsToNminusOne(
      const std::vector<PolyominoSubsetIndex> &candidate_tiles,
      BitMaskType current_state = 0,
      std::size_t current_index = 0) const noexcept;

  const PuzzleParams &params;
};

void PreProcessConfiguration(std::vector<PolyominoSubsetIndex> &p);

template <std::size_t N>
std::string PuzzleSolver::decodeSolution(
    Polyomino<N> board, const std::vector<std::size_t> &solution,
    const std::vector<PolyominoSubsetIndex> &candidate_tiles) const noexcept {
  if (solution.size() != candidate_tiles.size()) {
    return "No solution found";
  }
  board = board.sorted();
  const auto [x_max, y_max] = board.max_xy();
  std::stringstream result;
  for (int y = 0; y <= y_max; ++y) {
    for (int x = 0; x <= x_max; ++x) {
      auto idx = board.find_coord({x, y});
      if (!idx) {
        result << ".";
        continue;
      }
      bool found = false;
      for (std::size_t i = 0; i < candidate_tiles.size(); ++i) {
        const auto &tile = params[candidate_tiles[i]][solution[i]];
        if (tile & (1 << *idx)) {
          found = true;
          result << kColors[i % kColors.size()];
          break;
        }
      }
      if (!found) {
        result << "?";
      }
    }
    result << "\n";
  }
  return result.str();
}
