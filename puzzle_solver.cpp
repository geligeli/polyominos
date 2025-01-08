#include "puzzle_solver.hpp"
#include "dl_matrix.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <bit>
#include <bits/chrono.h>
#include <bits/utility.h>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <optional>
#include <vector>

template <int N> struct PrecomputedPolyminosMatchSet {

  // template <std::size_t BOARD_SIZE>
  // static const std::function<
  //     std::vector<BitMaskType>(const Polyomino<BOARD_SIZE> &, int)> &
  // cpp_matchers() {
  //   static const std::function<std::vector<BitMaskType>(
  //       const Polyomino<BOARD_SIZE> &, int)>
  //       val = [](const Polyomino<BOARD_SIZE> &board, int i) {
  //         return FindMatchPatterns(board,
  //                                  PrecomputedPolyminosSet<N>::polyminos()[i]);
  //       };
  //   return val;
  // }

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

  static const std::vector<std::vector<std::pair<int8_t, int8_t>>> &xy_cords_vector() {
    static const  std::vector<std::vector<std::pair<int8_t, int8_t>>> val = []() {
      const auto &ps = PrecomputedPolyminosSet<N>::polyminos();
      std::vector<std::vector<std::pair<int8_t, int8_t>>> result(ps.size());
      std::size_t i = 0;
      for (const auto &p : ps) {
        result[i++] = p.xy_cords_vector();
      }
      return result;
    }();
    return val;
  }
};

namespace {
template <typename T, T... ints>
auto _MatchConstructor(std::integer_sequence<T, ints...> int_seq) {
    return std::array<std::vector<CandidateMatchBitmask>, int_seq.size()>{
  {PrecomputedPolyminosMatchSet<ints+1>::matchers()...} };
};

template <typename T, T... ints>
auto _XyCordConstructor(std::integer_sequence<T, ints...> int_seq) {
    return std::array<std::vector<std::vector<std::pair<int8_t, int8_t>>>, int_seq.size()>{
  {PrecomputedPolyminosMatchSet<ints+1>::xy_cords_vector()...} };
};
}

const std::array<std::vector<CandidateMatchBitmask>, kMaxPolyominoSize>
    kPrecomputedPolyminosMatchSet = _MatchConstructor(std::make_integer_sequence<int, kMaxPolyominoSize>{});

const std::array<std::vector<std::vector<std::pair<int8_t, int8_t>>>, kMaxPolyominoSize>
    kPrecomputedPolyominosTypeErased = _XyCordConstructor(std::make_integer_sequence<int, kMaxPolyominoSize>{});

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


const std::vector<std::pair<int8_t, int8_t>> &
PuzzleParams::xy_coordinates(PolyominoSubsetIndex idx) const noexcept {
  const auto global_idx =
      possible_tiles_per_size[idx.N - 1][idx.index].polyomino_index;
  return kPrecomputedPolyominosTypeErased[global_idx.N - 1][global_idx.index];
}

PuzzleSolver::PuzzleSolver(const PuzzleParams &params) : params(params) {}

bool PuzzleSolver::Solve(
    const std::vector<PolyominoSubsetIndex> &candidate_tiles,
    std::vector<std::size_t> &solution, Algoritm algo) const noexcept {
  if (algo != Algoritm::DLX) {
    std::vector<uint64_t> v;
    uint64_t tile_marker = uint64_t{1} << params.N;
    std::vector<std::size_t> row_idx_to_mask_index_of_tile;
    for (const auto &tile : candidate_tiles) {
      std::size_t sol_idx = 0;
      for (const auto l : params[tile]) {
        uint64_t line = l;
        line |= tile_marker;
        v.push_back(line);
        row_idx_to_mask_index_of_tile.push_back(sol_idx);
        ++sol_idx;
      }
      tile_marker <<= 1;
    }
    std::vector<std::size_t> rows;
    DLMatrix dl_matrix(v);
    if (SolveCoverProblem(dl_matrix, rows)) {
      solution.resize(candidate_tiles.size());
      for (const auto row_idx : rows) {
        const auto r = v[row_idx];
        const auto tile_index = std::countr_zero(r >> params.N);
        solution[tile_index] = row_idx_to_mask_index_of_tile[row_idx];
      }
      return true;
    }
  } else if (algo == Algoritm::BF) {
    solution.resize(candidate_tiles.size());
    if (!internalSolve(candidate_tiles, solution)) {
      solution.clear();
      return false;
    }
    return true;
  }
  return false;
}

bool PuzzleSolver::internalSolve(
    const std::vector<PolyominoSubsetIndex> &candidate_tiles,
    std::vector<std::size_t> &indices, BitMaskType current_state,
    std::size_t current_index) const noexcept {
  if (current_index == candidate_tiles.size()) {
    return true;
  }
  const auto &cur_params = params[candidate_tiles[current_index]];
  std::size_t start_offset = 0;
  if (current_index > 0 &&
      candidate_tiles[current_index - 1] == candidate_tiles[current_index]) {
    start_offset = indices[current_index - 1] + 1;
  }
  for (std::size_t i = start_offset; i < cur_params.size(); ++i) {
    const auto mask = cur_params[i];
    if ((current_state & mask) != 0) {
      continue;
    }
    indices[current_index] = i;
    if (internalSolve(candidate_tiles, indices, current_state | mask,
                      current_index + 1)) {
      return true;
    }
  }
  return false;
};

// double PuzzleSolver::EstimateDifficulty(
//     const std::vector<PolyominoSubsetIndex> &candidate_tiles,
//     Algoritm algo) const noexcept {

//   uint64_t num_before_solution = 0;
//   uint64_t num_solutions = 0;

//   std::vector<std::size_t> num_before_last_per_tile(candidate_tiles.size());
//   // std::vector<std::size_t> num_solution_per_tile(candidate_tiles.size());

//   std::vector<std::size_t> next_duplicate_element(candidate_tiles.size(), 0);
//   std::vector<std::size_t> candidate_start_idx(candidate_tiles.size());

//   for (std::size_t i = 1; i < candidate_tiles.size(); ++i) {
//     if (candidate_tiles[i - 1] == candidate_tiles[i]) {
//       next_duplicate_element[i - 1] = i;
//       candidate_start_idx[i] = params[candidate_tiles[i]].size();
//     }
//   }
//   std::vector<std::size_t> indices(candidate_tiles.size());
//   auto walkSolutions = [&](auto &&self, BitMaskType current_state,
//                            std::size_t current_index) -> void {
//     if (current_index == candidate_tiles.size()) {
//       num_solutions++;
//       return;
//     }
//     // std::size_t start_offset = 0;
//     // bool terminal = true;
//     for (std::size_t tile_idx = 0; tile_idx < candidate_tiles.size();
//          ++tile_idx) {

//       const auto &cur_params = params[candidate_tiles[tile_idx]];
//       if (candidate_start_idx[tile_idx] == cur_params.size()) {
//         continue;
//       }
//       std::size_t start_offset =
//           std::exchange(candidate_start_idx[tile_idx], cur_params.size());

//       if (current_index == candidate_tiles.size() - 1 &&
//           start_offset < cur_params.size()) {
//         num_before_last_per_tile[tile_idx]++;
//       }

//       for (std::size_t i = start_offset; i < cur_params.size(); ++i) {
//         const auto mask = cur_params[i];
//         if ((current_state & mask) != 0) {
//           continue;
//         }

//         indices[tile_idx] = i;
//         if (next_duplicate_element[tile_idx] != 0) {
//           if (candidate_start_idx[next_duplicate_element[tile_idx]] !=
//               cur_params.size()) {
//             std::cerr << "Logic error " << std::endl;
//             std::abort();
//           }
//           candidate_start_idx[next_duplicate_element[tile_idx]] = i + 1;
//         }
//         self(self, current_state | mask, current_index + 1);
//         if (next_duplicate_element[tile_idx] != 0) {
//           candidate_start_idx[next_duplicate_element[tile_idx]] =
//               cur_params.size();
//         }
//       }
//       candidate_start_idx[tile_idx] = start_offset;
//     }
//   };

//   walkSolutions(walkSolutions, 0, 0);
//   num_before_solution = *std::min_element(num_before_last_per_tile.begin(),
//                                           num_before_last_per_tile.end());
//   return std::log2(num_before_solution) - std::log2(num_solutions);
// }

double PuzzleSolver::EstimateDifficulty(
    const std::vector<PolyominoSubsetIndex> &candidate_tiles,
    Algoritm algo) const noexcept {

  const auto calc_difficulty =
      [&params =
           params](const std::vector<PolyominoSubsetIndex> &candidate_tiles,
                   uint64_t num_before_solution_cutoff =
                       std::numeric_limits<uint64_t>::max()) {
        uint64_t num_before_solution = 0;
        uint64_t num_solutions = 0;
        std::vector<std::size_t> previous_dup_elem(candidate_tiles.size(), 0);
        for (std::size_t i = 1; i < candidate_tiles.size(); ++i) {
          for (std::size_t j = 0; j < i; ++j) {
            if (candidate_tiles[j] == candidate_tiles[i]) {
              previous_dup_elem[i] = j;
            }
          }
        }
        std::vector<std::size_t> indices(candidate_tiles.size());
        auto walkSolutions = [&](auto &&self, BitMaskType current_state,
                                 std::size_t current_index) -> void {
          const auto &cur_params = params[candidate_tiles[current_index]];
          uint64_t start_offset = 0;
          if (previous_dup_elem[current_index] != 0) {
            start_offset = indices[previous_dup_elem[current_index]] + 1;
          }
          if (current_index == candidate_tiles.size() - 1) {
            ++num_before_solution;
          }
          if (num_before_solution > num_before_solution_cutoff) {
            return;
          }
          for (std::size_t i = start_offset; i < cur_params.size(); ++i) {
            const auto mask = cur_params[i];
            if ((current_state & mask) != 0) {
              continue;
            }
            if (current_index == candidate_tiles.size() - 1) {
              ++num_solutions;
            } else {
              indices[current_index] = i;
              self(self, current_state | mask, current_index + 1);
              if (num_before_solution > num_before_solution_cutoff) {
                return;
              }
            }
          }
        };
        walkSolutions(walkSolutions, 0, 0);
        return std::make_pair(num_before_solution, num_solutions);
      };

  std::vector<PolyominoSubsetIndex> c = candidate_tiles;
  auto [num_before_solution, num_solutions] = calc_difficulty(c);

  for (std::size_t i = 1; i < c.size(); ++i) {
    std::rotate(c.begin(), c.begin() + 1, c.end());
    num_before_solution = std::min(
        num_before_solution, calc_difficulty(c, num_before_solution).first);
  }
  return std::log2(num_before_solution) - std::log(num_solutions);
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
