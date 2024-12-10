#include "loggers.hpp"
#include "partition_function.hpp"
#include "polyominos.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <execution>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <mutex>
#include <numeric>
#include <optional>
#include <set>
#include <thread>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

struct SolutionStats {
  int64_t possibilities_at_last_step{};
  int64_t solution_count{};

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
  // std::vector<std::array<int, N>> solutions;
};

template <int N, PolyominoConcept... Tiles> struct PuzzleParams {
  static constexpr auto kNumTiles = sizeof...(Tiles);
  std::array<std::vector<int64_t>, sizeof...(Tiles)> params;
  std::tuple<Tiles...> tiles;
  Polyomino<N> board;

  template <PolyominoConcept NextTile>
  constexpr std::optional<PuzzleParams<N, NextTile, Tiles...>>
  operator()(NextTile next_tile) const {
    auto next_param = FindMatchPatterns(board, next_tile);
    if (next_param.empty()) {
      return std::nullopt;
    }
    std::array<std::vector<int64_t>, sizeof...(Tiles) + 1> new_params;
    std::copy(params.begin(), params.end(), new_params.begin() + 1);
    new_params[0] = next_param;
    return PuzzleParams<N, NextTile, Tiles...>{
        new_params, std::tuple_cat(std::make_tuple(next_tile), tiles), board};
  }

  void print() const {
    std::cout << "Board: " << std::endl;
    board.print();
    std::cout << "Tiles: " << std::endl;
    std::apply([](auto &&...args) { ((args.print()), ...); }, tiles);
  }
};

template <int N, PolyominoConcept... Tiles> class PuzzleSolver {
public:
  SolutionStats &stats;
  const PuzzleParams<N, Tiles...> &puzzle_params;
  std::array<int, sizeof...(Tiles)> indices;
  std::array<int, sizeof...(Tiles)> foundSolution;
  static constexpr auto kNumTiles = sizeof...(Tiles);
  bool earlyAbort{false};

  std::string decodeSolution() const {
    const auto [x_max, y_max] = puzzle_params.board.max_xy();
    std::stringstream result;
    for (int y = 0; y <= y_max; ++y) {
      for (int x = 0; x <= x_max; ++x) {
        auto idx = puzzle_params.board.find_coord({x, y});
        if (!idx) {
          result << ".";
          continue;
        }
        bool found = false;
        for (int i = 0; i < kNumTiles; ++i) {
          const auto &tile = puzzle_params.params[i][foundSolution[i]];
          if (tile & (1 << *idx)) {
            found = true;
            result << i;
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

  template <int CURRENT_INDEX = 0>
  inline void Solve(int64_t current_state = 0) noexcept {
    if (earlyAbort) {
      return;
    }
    if constexpr (CURRENT_INDEX == kNumTiles) {
      if (stats.solution_count == 0) {
        foundSolution = indices;
      }
      ++stats.solution_count;
      if (stats.solution_count > 1) {
        earlyAbort = true;
      }
      // stats.solutions.push_back(indices);
      // indices has solution.
      return;
    } else {
      if constexpr (CURRENT_INDEX == kNumTiles - 1) {
        ++stats.possibilities_at_last_step;
      }
      for (int i = 0; i < puzzle_params.params[CURRENT_INDEX].size(); ++i) {
        const auto mask = puzzle_params.params[CURRENT_INDEX][i];
        if ((current_state & mask) != 0) {
          continue;
        }
        indices[CURRENT_INDEX] = i;
        Solve<CURRENT_INDEX + 1>(current_state | mask);
        if (earlyAbort) {
          return;
        }
      }
    }
  };

  template <int CURRENT_INDEX = 0>
  inline int64_t
  CountSolutionsToNminusOne(int64_t current_state = 0) const noexcept {
    if constexpr (CURRENT_INDEX == kNumTiles - 1) {
      return 1;
    } else {
      int64_t result = 0;
      for (int i = 0; i < puzzle_params.params[CURRENT_INDEX].size(); ++i) {
        const auto mask = puzzle_params.params[CURRENT_INDEX][i];
        if ((current_state & mask) != 0) {
          continue;
        }
        result +=
            CountSolutionsToNminusOne<CURRENT_INDEX + 1>(current_state | mask);
      }
      return result;
    }
  };
};

template <int N> struct PuzzleBoard {
  Polyomino<N> board;
  template <PolyominoConcept NextTile>
  constexpr std::optional<PuzzleParams<N, NextTile>>
  operator()(NextTile next_tile) const {
    auto next_param = FindMatchPatterns(board, next_tile);
    if (next_param.empty()) {
      return std::nullopt;
    }
    return PuzzleParams<N, NextTile>{
        std::array<std::vector<int64_t>, 1>{next_param},
        std::tuple<NextTile>(next_tile), board};
  }
};

std::vector<int64_t> CrossProduct(const std::vector<int64_t> &a,
                                  const std::vector<int64_t> &b) {
  std::vector<int64_t> result;
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

template <int N, PolyominoConcept... Tiles>
void PreProcessConfiguration(
    PuzzleParams<N, Tiles...> &puzzle_configuration) noexcept {
  // template <int N, PolyominoConcept... Tiles>
  // SolutionStats SolvePuzzle(
  //     PuzzleParams<N, Tiles...>& puzzle_configuration) noexcept {
  //   SolutionStats stats;

  auto &params = puzzle_configuration.params;

  std::sort(params.begin(), params.end());

  // Compactify the params
  auto start = params.begin();
  while (start != params.end()) {
    auto end = std::upper_bound(start, params.end(), *start);
    if (end == params.end()) {
      ++start;
      continue;
    }
    for (auto it = std::next(start); it != end; ++it) {
      *start = CrossProduct(*start, *it);
      *it = {0};
    }
    start = end;
  }

  // Order by size
  std::sort(params.begin(), params.end(),
            [](const auto &a, const auto &b) { return a.size() < b.size(); });

  // PuzzleSolver<puzzle_configuration::kNumTiles> solver{stats,
  // puzzle_configuration.params};
}

template <int N, int M> inline constexpr Polyomino<N * M> CreateRectangle() {
  std::array<std::pair<int8_t, int8_t>, N * M> coords;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      coords[i * M + j] = {i, j};
    }
  }
  return Polyomino<N * M>{coords}.canonical();
}

template <int N> inline constexpr Polyomino<N * N> CreateSquare() {
  return CreateRectangle<N, N>();
}

template <int N> struct PrecomputedPolyminosSet {
  static const auto &polyminos() {
    static const auto val = []() {
      auto result = get_next_gen(PrecomputedPolyminosSet<N - 1>::polyminos());
      std::cout << "Generated polyminos for N=" << N << std::endl;
      return result;
    }();
    return val;
  }
};

template <> struct PrecomputedPolyminosSet<1> {
  static const auto &polyminos() {
    static const auto val = []() {
      Polyomino<1> n0;
      n0.xy_cords[0].first = 0;
      n0.xy_cords[0].second = 0;
      return std::vector<Polyomino<1>>{n0};
    }();
    return val;
  }
};

template <std::size_t N, std::array<int, N> A>
constexpr std::array<int, N - 1> peel() {
  std::array<int, N - 1> result;
  for (int i = 0; i < N - 1; ++i) {
    result[i] = A[i + 1];
  }
  return result;
}

template <std::size_t N>
constexpr bool AcceptPartition(const std::array<int, N> &partition) {
  if (partition[0] == N) {
    return false;
  }
  return (partition[0] <= 5);
  // int partition_size = 0;
  // int last_p = partition[0];
  // int number_of_ones = 0;
  // for (auto p : partition) {
  //   if (p == 0) {
  //     break;
  //   }
  //   if (p == 1) {
  //     ++number_of_ones;
  //   }
  //   ++partition_size;
  //   last_p = p;
  // }
  // return partition_size > 5 && last_p > 2;
  // return number_of_ones < 4;
}

template <int N, std::array<int, N> PARTITION_OF_N, std::size_t CUR_INDEX = 0>
void TryAllPossibilitiesForPartition(const Polyomino<N> &board,
                                     auto &&param_generator,
                                     const auto &puzzle_configuration_visitor) {
  if constexpr (!AcceptPartition(PARTITION_OF_N)) {
    return;
  } else {
    constexpr int kRemainder =
        N - std::accumulate(PARTITION_OF_N.begin(),
                            PARTITION_OF_N.begin() + CUR_INDEX, 0);
    constexpr int kCurrentPolyominoSizeToConsider = PARTITION_OF_N[CUR_INDEX];

    if constexpr (kRemainder != kCurrentPolyominoSizeToConsider) {
      static_assert(kRemainder > 0);
      static_assert(kCurrentPolyominoSizeToConsider > 0);
      static_assert(kRemainder > kCurrentPolyominoSizeToConsider);

      for (const auto &p : PrecomputedPolyminosSet<
               kCurrentPolyominoSizeToConsider>::polyminos()) {
        auto n = param_generator(p);
        if (!n) {
          continue;
        }
        TryAllPossibilitiesForPartition<N, PARTITION_OF_N, CUR_INDEX + 1>(
            board, *std::move(n), puzzle_configuration_visitor);
      }
    } else {
      static_assert(kRemainder > 0);
      for (const auto &p : PrecomputedPolyminosSet<
               kCurrentPolyominoSizeToConsider>::polyminos()) {
        auto n = param_generator(p);
        if (!n) {
          continue;
        }
        puzzle_configuration_visitor(*std::move(n));
      }
    }
  }
}

template <int N, int... ints>
void _ForAllPartitions(
    const Polyomino<N> &board, const auto &puzzle_configuration_visitor,
    [[maybe_unused]] std::integer_sequence<int, ints...> ints_sequence) {

  (TryAllPossibilitiesForPartition<N, Partition<N>::kPartition[ints]>(
       board, PuzzleBoard(board), puzzle_configuration_visitor),
   ...);
}

template <int N>
void ForAllPartitions(const Polyomino<N> &board,
                      const auto &puzzle_configuration_visitor) {
  _ForAllPartitions(board, puzzle_configuration_visitor,
                    std::make_integer_sequence<int, kNumberOfPartitions[N]>());
}

int main() {
  std::mutex m;
  SolutionStats best_stats{};
  std::atomic<int64_t> puzzle_configs = 0;

  std::atomic<int> numBoardsSolved = 0;
  PeriodicLogger log([&]() {
    std::cout << "\rBoards solved: " << numBoardsSolved << std::flush;
  });

  auto puzzle_configuration_visitor = [&]<int N, PolyominoConcept... Tiles>(
      PuzzleParams<N, Tiles...> && puzzle_configuration) {
    thread_local static auto puzzleTimer =
        TimingLogger::instance().getTimer("SolvePuzzle");
    puzzleTimer.tic();

    PreProcessConfiguration(puzzle_configuration);
    SolutionStats stats{};
    PuzzleSolver solver{stats, puzzle_configuration};
    solver.Solve();
    ++puzzle_configs;
    puzzleTimer.toc();
    if (solver.earlyAbort || solver.stats.solution_count == 0) {
      return;
    }
    auto sampleSolution = solver.decodeSolution();

    thread_local static auto solutionCounterTimer =
        TimingLogger::instance().getTimer("SolutionCounter");
    solutionCounterTimer.tic();
    const auto cutoff = [&]() {
      std::lock_guard lk(m);
      return best_stats.possibilities_at_last_step;
    }();

    for (int i = 1; i < puzzle_configuration.params.size(); ++i) {
      if (stats.possibilities_at_last_step < cutoff) {
        break;
      }
      std::rotate(puzzle_configuration.params.begin(),
                  puzzle_configuration.params.begin() + 1,
                  puzzle_configuration.params.end());
      stats.possibilities_at_last_step = std::min(
          stats.possibilities_at_last_step, solver.CountSolutionsToNminusOne());
    }
    solutionCounterTimer.toc();

    std::lock_guard lk(m);
    if (stats > best_stats) {
      best_stats = stats;
      std::cout << "\nSolutions: " << stats.solution_count << std::endl;

      std::cout << "Possibilities at last step: "
                << stats.possibilities_at_last_step << std::endl;
      std::cout << "Puzzle: " << std::endl;
      puzzle_configuration.board.print();
      std::apply([](auto &&...args) { ((args.print()), ...); },
                 puzzle_configuration.tiles);
      std::cout << "Solution: \n";
      std::cout << sampleSolution << "\n";
    }
  };

  const auto visitor = [&](const auto &board) {
    ++numBoardsSolved;
    ForAllPartitions(board, puzzle_configuration_visitor);
  };

  const auto &kBoards = PrecomputedPolyminosSet<16>::polyminos();
  std::for_each(std::execution::par_unseq, kBoards.begin(), kBoards.end(),
                visitor);

  std::cout << "\nPuzzles solved=" << puzzle_configs << "\n";

  return 0;
}
