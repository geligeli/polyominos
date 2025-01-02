#pragma once
#include "avx_match.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>

constexpr std::size_t kMaxPolyominoSize = 12;
extern const std::array<std::vector<CandidateMatchBitmask>, kMaxPolyominoSize>
    kPrecomputedPolyminosMatchSet;
extern const std::array<std::vector<std::string>, kMaxPolyominoSize>
    kPrecomputedPolyominosAsString;
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
  inline constexpr std::strong_ordering operator<=>(
      const PolyominoIndex &other) const noexcept = default;
};

struct PolyominoSubsetIndex {
  std::size_t N;
  std::size_t index;
  inline constexpr std::strong_ordering operator<=>(
      const PolyominoSubsetIndex &other) const noexcept = default;
};

using BitMaskType = uint32_t;

struct PuzzleParams {
  template <std::size_t N>
  explicit PuzzleParams(const Polyomino<N> &board) noexcept : N(N) {
    BoardMatcher matcher = PolyominoToBoardMatcher(board);
    for (std::size_t i = 0; i < kPrecomputedPolyminosMatchSet.size(); ++i) {
      for (std::size_t j = 0; j < kPrecomputedPolyminosMatchSet[i].size();
           ++j) {
        auto result =
            find_matches_avx512(matcher, kPrecomputedPolyminosMatchSet[i][j]);
        std::vector<BitMaskType> result_masks(result.begin(), result.end());
        if (result.size() > 0) {
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

  const std::vector<BitMaskType> &operator[](PolyominoSubsetIndex idx) const noexcept;
  const std::string &display_string(PolyominoSubsetIndex idx) const noexcept;

  std::size_t N;
  std::array<std::vector<Tile>, kPrecomputedPolyminosMatchSet.size()>
      possible_tiles_per_size;
};

class PuzzleSolver {
public:
  PuzzleSolver(const PuzzleParams &params);
  const PuzzleParams &params;

  struct SolvingState {
    SolutionStats stats{};
    bool earlyAbort{false};

    std::vector<PolyominoSubsetIndex> candidate_tiles;
    std::vector<std::size_t> indices;
    std::vector<std::size_t> foundSolution;
  };

  void Solve(SolvingState &state, BitMaskType current_state = 0,
             std::size_t current_index = 0) const noexcept;

  uint64_t CountSolutionsToNminusOne(
      const std::vector<PolyominoSubsetIndex> &candidate_tiles,
      BitMaskType current_state = 0,
      std::size_t current_index = 0) const noexcept;

  template <std::size_t N>
  std::string decodeSolution(Polyomino<N> board,
                             const SolvingState &state) const noexcept {
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
        for (std::size_t i = 0; i < state.candidate_tiles.size(); ++i) {
          const auto &tile =
              params[state.candidate_tiles[i]][state.foundSolution[i]];
          if (tile & (1 << *idx)) {
            found = true;
            result << kColors[i];
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

};

std::vector<BitMaskType> CrossProduct(const std::vector<BitMaskType> &a,
                                      const std::vector<BitMaskType> &b);
void PreProcessConfiguration(std::vector<PolyominoSubsetIndex> &p);