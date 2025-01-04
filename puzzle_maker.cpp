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

  std::size_t tile_start_idx = 0;
  if (current_index > 0 && kCurrentPolyominoSizeToConsider == partition[current_index-1]) {
    tile_start_idx = p.back().index;
  }

  if (kRemainder != kCurrentPolyominoSizeToConsider) {
    for (std::size_t i = tile_start_idx; i < tiles.size(); ++i) {
      PolyominoSubsetIndex tile{kCurrentPolyominoSizeToConsider, i};
      p.push_back(tile);
      TryAllPossibilitiesForPartition(
          params, partition, puzzle_configuration_visitor, current_index + 1, p,
          squares_covered_so_far + kCurrentPolyominoSizeToConsider);
      p.pop_back();
    }
  } else {
    for (std::size_t i = tile_start_idx; i < tiles.size(); ++i) {
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
  constexpr int N = 30;

  std::vector<std::size_t> candidate_set;

  // const auto &ps = PrecomputedPolyminosSet<N>::polyminos();

  std::array<Polyomino<30>,1> ps = {CreateRectangle<6, 5>()};


  for (std::size_t idx = 0; idx < ps.size(); ++idx) {
    // if (ps[idx].num_symmetries() > 1) {
    //   continue;
    // }
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

  thread_local double tl_most_difficult{};

  std::mutex best_stats_mutex;
  double most_difficult{};

  std::mutex count_mutex;
  int i = 0;
  uint64_t skipped_partitions{};
  std::chrono::steady_clock::time_point start_time =
      std::chrono::steady_clock::now();
  std::for_each(
      // std::execution::par_unseq,
      candidate_set.begin(), candidate_set.end(),
      [&](const auto &polyomino) {
        PuzzleParams params{ps[polyomino]};
        for (const auto &partition : partitions) {
          auto possibilities =
              EnumerateAllPossibilitiesForPartition(params, partition);
          std::for_each(
              std::execution::par_unseq,
              possibilities.begin(), possibilities.end(),
              [&](std::vector<PolyominoSubsetIndex> &p) {
                PreProcessConfiguration(p);
                if (params.possibilities_for_configuration(p) > 50) {
                  ++skipped_partitions;
                  std::cout
                      << "Skipping configuration with too many possibilities"
                      << std::endl;
                  return;
                }

                PuzzleSolver solver(params);
                std::vector<std::size_t> solution;

                if (!solver.Solve(p, solution)) {
                  return;
                }
                double difficulty = solver.EstimateDifficulty(p);

                if (difficulty > tl_most_difficult) {
                  tl_most_difficult = difficulty;

                  std::lock_guard lk(best_stats_mutex);
                  if (difficulty > most_difficult) {
                    most_difficult = difficulty;
                    std::cout << "\nDifficulty " << difficulty << "\n";
                    std::cout << "Decoded solution: \n";
                    std::cout
                        << solver.decodeSolution(ps[polyomino], solution, p)
                        << "\n";
                    std::cout << "Partition=";
                    for (auto t : partition) {
                      std::cout << t << " ";
                    }
                    std::cout << std::endl;
                  } else {
                    tl_most_difficult = most_difficult;
                  }
                }
              });
        }
        {
          std::lock_guard lk(count_mutex);
          ++i;
          if ((i & 0xff) == 0) {
            auto dt = std::chrono::steady_clock::now() - start_time;
            std::cout << "Polyomino: " << std::setw(8) << ++i << " of "
                      << candidate_set.size() << " "
                      << "Polyominos per second: " << std::setw(8)
                      << i / std::chrono::duration<double>(dt).count()
                      << " ETA: " << std::setw(8)
                      << std::chrono::duration<double>(dt).count() / i *
                             (candidate_set.size() - i)
                      << "s " << std::setw(4) << skipped_partitions
                      << " skipped configurations"
                      << "\r";
          }
        }
      });
  std::cout << "\nDone\n";

  return 0;
}
