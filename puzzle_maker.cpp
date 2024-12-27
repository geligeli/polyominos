#include "avx_match.hpp"
#include "loggers.hpp"
#include "partition_function.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstring>
#include <execution>
#include <iostream>
#include <mutex>
#include <optional>
#include <set>
#include <utility>
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

constexpr std::size_t kMaxPolyominoSize = 12;

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

struct SolutionStats {
  uint64_t possibilities_at_last_step{1};
  uint64_t solution_count{0};

  inline constexpr int64_t last_step_difficulty() const {
    return possibilities_at_last_step - solution_count;
  }

  inline constexpr std::weak_ordering
  operator<=>(const SolutionStats &other) const {
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
};

struct PolyominoIndex {
  std::size_t N;
  std::size_t index;
};

struct PolyominoSubsetIndex {
  std::size_t N;
  std::size_t index;
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

  uint64_t
  possibilities_for_partition(const std::vector<int> &partition) const {
    uint64_t result = 1;
    for (const auto &p : partition) {
      result *= possible_tiles_per_size[p - 1].size();
    }
    return result;
  }

  double possibilities_for_configuration(
      const std::vector<PolyominoSubsetIndex> &configuration) const {
    double result = 0;
    for (const auto &p : configuration) {
      result += std::log2(operator[](p).size());
    }
    return result;
  }

  inline const std::vector<BitMaskType> &
  operator[](PolyominoSubsetIndex idx) const noexcept {
    return possible_tiles_per_size[idx.N - 1][idx.index].masks;
  }

  inline const std::string &
  display_string(PolyominoSubsetIndex idx) const noexcept {
    const auto global_idx =
        possible_tiles_per_size[idx.N - 1][idx.index].polyomino_index;
    return kPrecomputedPolyominosAsString[global_idx.N - 1][global_idx.index];
  }

  std::size_t N;
  std::array<std::vector<Tile>, kPrecomputedPolyminosMatchSet.size()>
      possible_tiles_per_size;
};

const std::array<std::string, 14> kColors = {
    "\033[31m0\033[0m", "\033[32m1\033[0m", "\033[33m2\033[0m",
    "\033[34m3\033[0m", "\033[35m4\033[0m", "\033[36m5\033[0m",
    "\033[37m6\033[0m", "\033[91m7\033[0m", "\033[92m8\033[0m",
    "\033[93m9\033[0m", "\033[94mA\033[0m", "\033[95mB\033[0m",
    "\033[96mC\033[0m", "\033[97mD\033[0m",
};

class PuzzleSolver {
public:
  PuzzleSolver(const PuzzleParams &params) : params(params) {}

  const PuzzleParams &params;

  struct SolvingState {
    SolutionStats stats{};
    bool earlyAbort{false};

    std::vector<PolyominoSubsetIndex> candidate_tiles;
    std::vector<std::size_t> indices;
    std::vector<std::size_t> foundSolution;
  };

  inline void Solve(SolvingState &state, BitMaskType current_state = 0,
                    std::size_t current_index = 0) noexcept {
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
      // stats.solutions.push_back(indices);
      // indices has solution.
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

  inline uint64_t CountSolutionsToNminusOne(
      const std::vector<PolyominoSubsetIndex> &candidate_tiles,
      BitMaskType current_state = 0,
      std::size_t current_index = 0) const noexcept {
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

inline bool AcceptPartition(const std::vector<int> &partition, int N) {
  if (partition[0] == N || partition[0] == 1) {
    return false;
  }
  if (partition[0] > kMaxPolyominoSize) {
    return false;
  }
  // if (partition.size() > 6) {
  //   return false;
  // }
  // if (partition.size() < 4) {
  //   return false;
  // }
  return true;
}

void TryAllPossibilitiesForPartition(const PuzzleParams &params,
                                     const std::vector<int> &partition,
                                     const auto &puzzle_configuration_visitor,
                                     std::size_t current_index = 0,
                                     std::vector<PolyominoSubsetIndex> p = {},
                                     std::size_t squares_covered_so_far = 0) {
  if (squares_covered_so_far >= params.N) {
    std::cerr << "Invalid partition" << std::endl;
    for (auto p : partition) {
      std::cerr << p << " ";
    }
    std::cerr << std::endl;
    std::abort();
  }
  const int kRemainder = params.N - squares_covered_so_far;
  const std::size_t kCurrentPolyominoSizeToConsider = partition[current_index];

  const auto &tiles =
      params.possible_tiles_per_size[kCurrentPolyominoSizeToConsider - 1];
  if (kRemainder != kCurrentPolyominoSizeToConsider) {
    assert(kRemainder > 0);
    assert(kRemainder > kCurrentPolyominoSizeToConsider);

    for (std::size_t i = 0; i < tiles.size(); ++i) {
      PolyominoSubsetIndex tile{kCurrentPolyominoSizeToConsider, i};
      p.push_back(tile);
      TryAllPossibilitiesForPartition(
          params, partition, puzzle_configuration_visitor, current_index + 1, p,
          squares_covered_so_far + kCurrentPolyominoSizeToConsider);
      p.pop_back();
    }
  } else {
    assert(kRemainder > 0);
    for (std::size_t i = 0; i < tiles.size(); ++i) {
      PolyominoSubsetIndex tile{kCurrentPolyominoSizeToConsider, i};
      p.push_back(tile);
      puzzle_configuration_visitor(p);
      p.pop_back();
    }
  }
}

std::vector<std::vector<PolyominoSubsetIndex>>
EnumerateAllPossibilitiesForPartition(const PuzzleParams &params,
                                      const std::vector<int> &partition

) {
  std::vector<std::vector<PolyominoSubsetIndex>> result;
  TryAllPossibilitiesForPartition(params, partition,
                                  [&](const auto &p) { result.push_back(p); });
  return result;
}

int main() {
  constexpr int N = 16;

  // std::vector<Polyomino<N>> candidate_set;
  // candidate_set.push_back(RemoveOne(CreateSquare<5>(), 1));

  std::vector<std::size_t> candidate_set;

  const auto &ps = PrecomputedPolyminosSet<N>::polyminos();

  for (std::size_t idx = 0; idx < ps.size(); ++idx) {
    if (ps[idx].num_symmetries() > 1) {
      continue;
    }
    candidate_set.push_back(idx);
  }

  std::sort(candidate_set.begin(), candidate_set.end(),
            [&](const auto &a, const auto &b) {
              auto [ax, ay] = ps[a].max_xy();
              auto [bx, by] = ps[b].max_xy();
              return std::max(ax, ay) < std::max(bx, by);
              // return a.bounding_box_area() < b.bounding_box_area();
            });

  std::vector<std::vector<int>> partitions = generate_partitions(N);
  partitions.erase(std::remove_if(partitions.begin(), partitions.end(),
                                  [](const std::vector<int> &partition) {
                                    return !AcceptPartition(partition, N);
                                  }),
                   partitions.end());

  thread_local SolutionStats tl_best_stats{};

  std::mutex best_stats_mutex;
  SolutionStats best_stats{};

  std::mutex count_mutex;
  int i = 0;

  // for (const auto &polyomino : candidate_set) {
  std::for_each(
      std::execution::par_unseq, candidate_set.begin(), candidate_set.end(),
      [&ps = std::as_const(ps), &partitions = std::as_const(partitions),
       &candidate_set = std::as_const(candidate_set), &i, &count_mutex,
       &best_stats, &best_stats_mutex](const auto &polyomino) {
        PuzzleParams params{ps[polyomino]};
        for (const auto &partition : partitions) {
          auto possibilities =
              EnumerateAllPossibilitiesForPartition(params, partition);
          std::for_each(
              possibilities.begin(), possibilities.end(),
              [&](std::vector<PolyominoSubsetIndex> &p) {
                PreProcessConfiguration(p);
                if (params.possibilities_for_configuration(p) > 50) {
                  std::cout
                      << "Skipping configuration with too many possibilities"
                      << std::endl;
                  return;
                }
                PuzzleSolver::SolvingState solving_state{};
                PuzzleSolver solver(params);

                solving_state.candidate_tiles = p;
                solver.Solve(solving_state);

                if (solving_state.earlyAbort ||
                    solving_state.stats.solution_count == 0) {
                  return;
                }
                if (solving_state.stats > tl_best_stats) {
                  // stats might be better than tl_best_stats
                  auto solution_p = p;
                  const auto cutoff = tl_best_stats.possibilities_at_last_step;
                  for (int i = 1; i < p.size(); ++i) {
                    std::rotate(p.begin(), p.begin() + 1, p.end());
                    solving_state.stats.possibilities_at_last_step =
                        std::min(solving_state.stats.possibilities_at_last_step,
                                 solver.CountSolutionsToNminusOne(p));
                    if (solving_state.stats.possibilities_at_last_step <=
                        cutoff) {
                      // we know stats are worse than tl_best_stats
                      return;
                    }
                  }
                  if (solving_state.stats <= tl_best_stats) {
                    std::cerr << "Error: stats <= tl_best_stats" << std::endl;
                    std::abort();
                    return;
                  }

                  tl_best_stats = solving_state.stats;
                  std::lock_guard lk(best_stats_mutex);
                  if (solving_state.stats > best_stats) {
                    best_stats = solving_state.stats;
                    std::cout << "\n";
                    std::cout
                        << "Solutions: " << solving_state.stats.solution_count
                        << std::endl;
                    std::cout << "Possibilities at last step: "
                              << solving_state.stats.possibilities_at_last_step
                              << std::endl;
                    std::cout << "Decoded solution: \n";
                    std::cout
                        << solver.decodeSolution(ps[polyomino], solving_state)
                        << "\n";
                    for (auto t : solution_p) {
                      std::cout << params.display_string(t);
                    }
                    std::cout << "Partition=";
                    for (auto t : partition) {
                      std::cout << t << " ";
                    }
                    std::cout << std::endl;
                  } else {
                    tl_best_stats = best_stats;
                  }
                }
              });
        }
        {
          std::lock_guard lk(count_mutex);
          i++;
          std::cout << "Polyomino: " << i << " of " << candidate_set.size()
                    << "\r";
        }
      });
  std::cout << "\nDone\n";

  return 0;
}
