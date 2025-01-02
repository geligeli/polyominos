#include "avx_match.hpp"
#include "partition_function.hpp"
#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <execution>
#include <iostream>
#include <mutex>
#include <optional>
#include <utility>
#include <vector>

inline bool AcceptPartition(const std::vector<int> &partition, int N) {
  if (partition[0] == N || partition[0] == 1) {
    return false;
  }
  if (partition[0] > kMaxPolyominoSize) {
    return false;
  }
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
  constexpr int N = 17;

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
