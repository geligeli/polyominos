#include "avx_match.hpp"
#include "combinatorics.hpp"
#include "partition_function.hpp"
#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <chrono>
#include <cmath>
#include <execution>
#include <iostream>
#include <iterator>
#include <map>
#include <mutex>
#include <optional>
#include <thread>
#include <utility>
#include <vector>

inline bool AcceptPartition(const std::vector<int> &partition, int N) {
  if (partition[0] == N || partition[0] == 1) {
    return false;
  }
  if (partition[0] > kMaxPolyominoSize) {
    return false;
  }
  // return std::count_if(partition.begin(), partition.end(), [](int i) { return i != 1; }) == 6;
  // return (partition.size() <= 4);
  return true;
}

inline bool AcceptConfiguration(const std::vector<PolyominoSubsetIndex> &p) {
  // for (std::size_t i = 1; i < p.size(); ++i) {
  //   if (p[i - 1] == p[i]) {
  //     return false;
  //   }
  // }
  return true;
}

int main() {
  constexpr int N = 17;
  const auto &ps = PrecomputedPolyminosSet<N>::polyminos();

  // std::cout << sizeof(Polyomino<16>) << std::endl;

  // constexpr int N = 30;
  // std::array ps = {RemoveOne(CreateRectangle<6, 5>(), 1)};
  // // std::array ps = {CreateRectangle<6, 5>()};
  // static int N = ps[0].size;

  std::vector<std::size_t> candidate_set;

  for (std::size_t idx = 0; idx < ps.size(); ++idx) {
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
  // last step difficulty bounded by size of the smallest piece.
  std::sort(
      partitions.begin(), partitions.end(),
      [](const auto &a, const auto &b) { return a.back() > b.back(); }
  );


  thread_local double tl_most_difficult{};

  std::mutex best_stats_mutex;
  double most_difficult{};

  std::mutex count_mutex;
  int i = 0;
  uint64_t skipped_partitions{};
  std::chrono::steady_clock::time_point start_time =
      std::chrono::steady_clock::now();
  std::for_each(
      std::execution::par_unseq,
      candidate_set.begin(), candidate_set.end(), [&](const auto &polyomino) {
        const PuzzleParams params{ps[polyomino]};
        for (const auto &partition : partitions) {
          std::map<int, int> partition_map;
          for (auto p : partition) {
            partition_map[p]++;
          }

          using RangeType = MultiSetsRange;
          std::vector<RangeType> sequences;

          for (const auto [size, count] : partition_map) {
            uint64_t num_tiles =
                params.possible_tiles_per_size[size - 1].size();
            sequences.push_back(
                RangeType{num_tiles, static_cast<uint64_t>(count)});
          }
          // need to reverse the order of the sequences because partition is
          // ordered from largest to smallest and the indices into the output of
          // SubSetsRangeProduct are in the opposite order.
          std::reverse(sequences.begin(), sequences.end());
          RangeProductRange<RangeType> r(std::move(sequences));
          std::for_each(
              //  std::execution::par_unseq,
               r.begin(), r.end(), [&](std::vector<uint64_t> idx) {
                std::vector<PolyominoSubsetIndex> p(idx.size());
                for (std::size_t i = 0; i < idx.size(); ++i) {
                  p[i] = PolyominoSubsetIndex{
                      static_cast<std::size_t>(partition[i]), idx[i]};
                }

                if (!AcceptConfiguration(p)) {
                  return;
                }

                PreProcessConfiguration(p);

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
                    std::cout << "\n";

                    std::cout << "game_board(";
                    std::cout << ps[polyomino].openscad_string() << ");\n";
                    int max_x = ps[polyomino].max_xy().first + 2;
                    for (const auto &idx : p) {
                      const auto &xyc = params.xy_coordinates(idx);

                      std::cout << "polyomino([";
                      int highest_x = 0;
                      for (const auto &[x, y] : xyc) {
                        std::cout << "[" << static_cast<int>(x) + max_x << ","
                                  << static_cast<int>(y) << "],";
                        highest_x =
                            std::max(highest_x, static_cast<int>(x) + max_x);
                      }
                      max_x = highest_x + 2;
                      std::cout << "]);\n";
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
