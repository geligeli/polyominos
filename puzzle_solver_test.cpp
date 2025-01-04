#include "polyominos.hpp"
#include "puzzle_solver.hpp"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <iostream>
#include <numeric>
#include <random>

class PuzzleSolverTest
    : public ::testing::TestWithParam<PuzzleSolver::Algoritm> {};

INSTANTIATE_TEST_SUITE_P(SolverTest, PuzzleSolverTest,
                         ::testing::Values(PuzzleSolver::Algoritm::BF,
                                           PuzzleSolver::Algoritm::DLX));

TEST_P(PuzzleSolverTest, SimpleSolve) {
  const auto square = CreateSquare<4>();
  PuzzleParams params{square};
  PuzzleSolver solver(params);

  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;

  for (int i = 0; i < 8; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{2, 0});
  }
  ASSERT_TRUE(solver.Solve(candidate_tiles, solution, GetParam()));
  std::cout << solver.decodeSolution(square, solution, candidate_tiles) << std::endl;
}

TEST_P(PuzzleSolverTest, Unsatisfiable) {
  const auto square = RemoveOne(RemoveOne(CreateSquare<4>(), 3), 1);
  PuzzleParams params{square};
  PuzzleSolver solver(params);

  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;
  for (int i = 0; i < square.size / 2; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{2, 0});
  }
  ASSERT_FALSE(solver.Solve(candidate_tiles, solution, GetParam()));
}

TEST_P(PuzzleSolverTest, LargeSquare) {
  const auto square = CreateRectangle<6, 5>();
  PuzzleParams params{square};
  PuzzleSolver solver(params);

  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;
  for (std::size_t i = 0; i < square.size / 5; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{5, i});
  }
  EXPECT_TRUE(solver.Solve(candidate_tiles, solution, GetParam()));
  std::cout << solver.decodeSolution(square, solution, candidate_tiles) << std::endl;
}

TEST(PuzzleSolver, TestDifficulty) {
  const auto square = CreateRectangle<6, 5>();
  PuzzleParams params{square};
  PuzzleSolver solver(params);

  std::vector<PolyominoSubsetIndex> candidate_tiles;
  std::vector<std::size_t> solution;
  for (std::size_t i = 0; i < square.size / 5; ++i) {
    candidate_tiles.push_back(PolyominoSubsetIndex{5, i});
  }
  std::cout << solver.EstimateDifficulty(candidate_tiles) << std::endl;
}