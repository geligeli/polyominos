#include "avx_match.hpp"
#include "combinatorics.hpp"
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
  return std::count_if(partition.begin(), partition.end(), [](int i) { return i != 1; }) <= 5;
  // return (partition.size() <= 5);
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
  // constexpr int N = 17;
  // const auto &ps = PrecomputedPolyminosSet<N>::polyminos();

  // std::cout << sizeof(Polyomino<16>) << std::endl;

  
  
  // constexpr int N = 29;
  std::array ps = { RemoveOne(RemoveOne(CreateRectangle<6, 5>(),4),0) };

  static const int N = ps[0].size;

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

          std::vector<std::vector<PolyominoSubsetIndex>> input(
              partition.size());
          for (std::size_t ii = 0; ii < partition.size(); ++ii) {
            const auto &tiles =
                params.possible_tiles_per_size[partition[ii] - 1];
            for (std::size_t j = 0; j < tiles.size(); ++j) {
              input[ii].push_back(PolyominoSubsetIndex{
                  static_cast<std::size_t>(partition[ii]), j});
            }
          }

          auto [begin, end] = make_cross_product_iterator(input);
          std::vector<std::vector<PolyominoSubsetIndex>> possibilities(
              begin, end);

          std::for_each(
              std::execution::par_unseq,
              possibilities.begin(), possibilities.end(),
              /* begin, end, */ [&](std::vector<PolyominoSubsetIndex> p) {
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
