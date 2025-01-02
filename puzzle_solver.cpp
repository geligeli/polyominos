#include "puzzle_solver.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <iostream>
#include <optional>
#include <vector>

template <int N> struct PrecomputedPolyminosMatchSet {
  static const std::vector<CandidateMatchBitmask> &matchers() {
    static const std::vector<CandidateMatchBitmask> val = []() {
      const auto &ps = PrecomputedPolyminosSet<N>::polyminos();
      std::vector<CandidateMatchBitmask> result(ps.size());
      std::size_t i = 0;
      for (const auto &p : ps) {
        PolyominoToMatchBitMask(p, result[i++]);
      }
      return result;
    }();
    return val;
  }
  static const std::vector<std::string> &strings() {
    static const std::vector<std::string> val = []() {
      const auto &ps = PrecomputedPolyminosSet<N>::polyminos();
      std::vector<std::string> result(ps.size());
      std::size_t i = 0;
      for (const auto &p : ps) {
        result[i++] = p.string();
      }
      return result;
    }();
    return val;
  }
};

const std::array<std::vector<CandidateMatchBitmask>, kMaxPolyominoSize>
    kPrecomputedPolyminosMatchSet{PrecomputedPolyminosMatchSet<1>::matchers(),
                                  PrecomputedPolyminosMatchSet<2>::matchers(),
                                  PrecomputedPolyminosMatchSet<3>::matchers(),
                                  PrecomputedPolyminosMatchSet<4>::matchers(),
                                  PrecomputedPolyminosMatchSet<5>::matchers(),
                                  PrecomputedPolyminosMatchSet<6>::matchers(),
                                  PrecomputedPolyminosMatchSet<7>::matchers(),
                                  PrecomputedPolyminosMatchSet<8>::matchers(),
                                  PrecomputedPolyminosMatchSet<9>::matchers(),
                                  PrecomputedPolyminosMatchSet<10>::matchers(),
                                  PrecomputedPolyminosMatchSet<11>::matchers(),
                                  PrecomputedPolyminosMatchSet<12>::matchers()};

const std::array<std::vector<std::string>, kMaxPolyominoSize>
    kPrecomputedPolyominosAsString{PrecomputedPolyminosMatchSet<1>::strings(),
                                   PrecomputedPolyminosMatchSet<2>::strings(),
                                   PrecomputedPolyminosMatchSet<3>::strings(),
                                   PrecomputedPolyminosMatchSet<4>::strings(),
                                   PrecomputedPolyminosMatchSet<5>::strings(),
                                   PrecomputedPolyminosMatchSet<6>::strings(),
                                   PrecomputedPolyminosMatchSet<7>::strings(),
                                   PrecomputedPolyminosMatchSet<8>::strings(),
                                   PrecomputedPolyminosMatchSet<9>::strings(),
                                   PrecomputedPolyminosMatchSet<10>::strings(),
                                   PrecomputedPolyminosMatchSet<11>::strings(),
                                   PrecomputedPolyminosMatchSet<12>::strings()};

const std::array<std::string, 14> kColors = {
    "\033[31m0\033[0m", "\033[32m1\033[0m", "\033[33m2\033[0m",
    "\033[34m3\033[0m", "\033[35m4\033[0m", "\033[36m5\033[0m",
    "\033[37m6\033[0m", "\033[91m7\033[0m", "\033[92m8\033[0m",
    "\033[93m9\033[0m", "\033[94mA\033[0m", "\033[95mB\033[0m",
    "\033[96mC\033[0m", "\033[97mD\033[0m",
};

int64_t SolutionStats::last_step_difficulty() const {
  return possibilities_at_last_step - solution_count;
}

std::weak_ordering
SolutionStats::operator<=>(const SolutionStats &other) const {
  if (solution_count == 0 && other.solution_count == 0) {
    return std::weak_ordering::equivalent;
  } else if (solution_count == 0 && other.solution_count != 0) {
    return std::weak_ordering::less;
  } else if (solution_count != 0 && other.solution_count == 0) {
    return std::weak_ordering::greater;
  } else if (solution_count < other.solution_count) {
    return std::weak_ordering::greater;
  } else if (solution_count > other.solution_count) {
    return std::weak_ordering::less;
  } else if (last_step_difficulty() > other.last_step_difficulty()) {
    return std::weak_ordering::greater;
  } else if (last_step_difficulty() < other.last_step_difficulty()) {
    return std::weak_ordering::less;
  }
  return std::weak_ordering::equivalent;
}

using BitMaskType = uint32_t;

uint64_t PuzzleParams::possibilities_for_partition(
    const std::vector<int> &partition) const {
  uint64_t result = 1;
  for (const auto &p : partition) {
    result *= possible_tiles_per_size[p - 1].size();
  }
  return result;
}

double PuzzleParams::possibilities_for_configuration(
    const std::vector<PolyominoSubsetIndex> &configuration) const {
  double result = 0;
  for (const auto &p : configuration) {
    result += std::log2(operator[](p).size());
  }
  return result;
}
const std::vector<BitMaskType> &
PuzzleParams::operator[](PolyominoSubsetIndex idx) const noexcept {
  return possible_tiles_per_size[idx.N - 1][idx.index].masks;
}
const std::string &
PuzzleParams::display_string(PolyominoSubsetIndex idx) const noexcept {
  const auto global_idx =
      possible_tiles_per_size[idx.N - 1][idx.index].polyomino_index;
  return kPrecomputedPolyominosAsString[global_idx.N - 1][global_idx.index];
}

PuzzleSolver::PuzzleSolver(const PuzzleParams &params) : params(params) {}

void PuzzleSolver::Solve(SolvingState &state, BitMaskType current_state,
                         std::size_t current_index) const noexcept {
  if (state.earlyAbort) {
    return;
  }
  if (current_index == 0) {
    state.indices.resize(state.candidate_tiles.size());
    state.earlyAbort = false;
  }
  if (current_index == state.candidate_tiles.size()) {
    if (state.stats.solution_count == 0) {
      state.foundSolution = state.indices;
    }
    ++state.stats.solution_count;
    if (state.stats.solution_count > 1) {
      state.earlyAbort = true;
    }
    return;
  }
  if (current_index + 1 == state.candidate_tiles.size()) {
    ++state.stats.possibilities_at_last_step;
  }
  const auto &cur_params = params[state.candidate_tiles[current_index]];

  for (std::size_t i = 0; i < cur_params.size(); ++i) {
    const auto mask = cur_params[i];
    if ((current_state & mask) != 0) {
      continue;
    }
    state.indices[current_index] = i;
    Solve(state, current_state | mask, current_index + 1);
    if (state.earlyAbort) {
      return;
    }
  }
};

uint64_t PuzzleSolver::CountSolutionsToNminusOne(
    const std::vector<PolyominoSubsetIndex> &candidate_tiles,
    BitMaskType current_state, std::size_t current_index) const noexcept {
  if (current_index + 1 == candidate_tiles.size()) {
    return 1;
  }
  uint64_t result = 0;
  const auto &cur_params = params[candidate_tiles[current_index]];
  for (std::size_t i = 0; i < cur_params.size(); ++i) {
    const auto mask = cur_params[i];
    if ((current_state & mask) != 0) {
      continue;
    }
    result += CountSolutionsToNminusOne(candidate_tiles, current_state | mask,
                                        current_index + 1);
  }
  return result;
};

std::vector<BitMaskType> CrossProduct(const std::vector<BitMaskType> &a,
                                      const std::vector<BitMaskType> &b) {
  std::vector<BitMaskType> result;
  result.reserve(a.size() * b.size());
  for (const auto &x : a) {
    for (const auto &y : b) {
      if ((x & y) == 0) {
        result.push_back(x | y);
      }
    }
  }
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

void PreProcessConfiguration(std::vector<PolyominoSubsetIndex> &p) {
  p.erase(std::remove_if(p.begin(), p.end(),
                         [](const auto &x) { return x.N == 1; }),
          p.end());
};
