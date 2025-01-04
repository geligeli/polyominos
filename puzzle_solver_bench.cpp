#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include <benchmark/benchmark.h>


void BM_SolveUnsatisfiable14(benchmark::State &state, PuzzleSolver::Algoritm algo) {
  const auto square = RemoveOne(RemoveOne(CreateSquare<4>(), 3), 1);
  PuzzleParams params{square};
  std::vector<std::size_t> solution;
  std::vector<PolyominoSubsetIndex> candidate_tiles;
  for (int i = 0; i < square.size / 2; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{2, 0});
  }
  PuzzleSolver solver(params);
  for (auto _ : state) {
    solver.Solve(candidate_tiles, solution, algo);
  }
}
BENCHMARK_CAPTURE(BM_SolveUnsatisfiable14, "BF", PuzzleSolver::Algoritm::BF);
BENCHMARK_CAPTURE(BM_SolveUnsatisfiable14, "DLX", PuzzleSolver::Algoritm::DLX);

void BM_SolveUnsatisfiable16(benchmark::State &state, PuzzleSolver::Algoritm algo) {
  const auto square = RemoveOne(RemoveOne(CreateRectangle<3,6>(),3),1);
  PuzzleParams params{square};
  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;
  for (int i = 0; i < square.size / 2; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{2, 0});
  }
  PuzzleSolver solver(params);
  for (auto _ : state) {
    solver.Solve(candidate_tiles, solution, algo);
  }
}
BENCHMARK_CAPTURE(BM_SolveUnsatisfiable16, "BF", PuzzleSolver::Algoritm::BF);
BENCHMARK_CAPTURE(BM_SolveUnsatisfiable16, "DLX", PuzzleSolver::Algoritm::DLX);

void BM_LargeSquare(benchmark::State &state, PuzzleSolver::Algoritm algo) {
  const auto square = CreateRectangle<6, 5>();
  PuzzleParams params{square};
  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;
  for (std::size_t i = 0; i < square.size / 5; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{5, i});
  }
  PuzzleSolver solver(params);
  for (auto _ : state) {
    solver.Solve(candidate_tiles, solution, algo);
  }
}
BENCHMARK_CAPTURE(BM_LargeSquare, "BF", PuzzleSolver::Algoritm::BF);
BENCHMARK_CAPTURE(BM_LargeSquare, "DLX", PuzzleSolver::Algoritm::DLX);

BENCHMARK_MAIN();
