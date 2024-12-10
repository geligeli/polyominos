#pragma once
#include "loggers.hpp"

#include <algorithm>
#include <array>
#include <bitset>
#include <cmath>
#include <cstring>
#include <execution>
#include <functional>
#include <numeric>
#include <optional>
#include <set>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

template <int N> struct alignas(64) Polyomino {
  static inline constexpr int size = N;
  std::array<std::pair<int8_t, int8_t>, N> xy_cords;

  inline constexpr Polyomino rotate_90() const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {xy_cords[i].second, -xy_cords[i].first};
    }
    return result;
  }

  inline constexpr Polyomino rotate_180() const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {-xy_cords[i].first, -xy_cords[i].second};
    }
    return result;
  }

  inline constexpr Polyomino rotate_270() const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {-xy_cords[i].second, xy_cords[i].first};
    }
    return result;
  }

  inline constexpr Polyomino flip_x() const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {-xy_cords[i].first, xy_cords[i].second};
    }
    return result;
  }

  inline constexpr Polyomino flip_y() const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {xy_cords[i].first, -xy_cords[i].second};
    }
    return result;
  }

  inline constexpr Polyomino flip_ac() const noexcept {
    return rotate_90().flip_x();
  }

  inline constexpr Polyomino flip_bd() const noexcept {
    return rotate_90().flip_y();
  }

  inline constexpr auto symmetries() const noexcept {
    if constexpr (N == 1) {
      return std::array<Polyomino, 1>{*this};
    } else if constexpr (N == 2) {
      return std::array<Polyomino, 2>{*this, rotate_90()};
    } else {
      return std::array<Polyomino, 8>{*this,        rotate_90(), rotate_180(),
                                      rotate_270(), flip_x(),    flip_y(),
                                      flip_ac(),    flip_bd()};
    }
  }

  inline constexpr Polyomino translate_xy(int8_t x, int8_t y) const noexcept {
    Polyomino result;
    for (int i = 0; i < N; ++i) {
      result.xy_cords[i] = {xy_cords[i].first + x, xy_cords[i].second + y};
    }
    return result;
  }

  inline constexpr Polyomino _align_to_positive_quadrant() const noexcept {
    int8_t x_min = xy_cords[0].first;
    int8_t y_min = xy_cords[0].second;
    for (int i = 1; i < N; ++i) {
      x_min = std::min(x_min, xy_cords[i].first);
      y_min = std::min(y_min, xy_cords[i].second);
    }
    return translate_xy(-x_min, -y_min);
  }

  inline constexpr Polyomino sorted() const noexcept {
    Polyomino result;
    result.xy_cords = xy_cords;
    std::sort(result.xy_cords.begin(), result.xy_cords.end());
    return result;
  }

  inline constexpr Polyomino canonical() const noexcept {
    Polyomino result = _align_to_positive_quadrant().sorted();
    for (auto p : symmetries()) {
      p = p._align_to_positive_quadrant().sorted();
      if (p < result) {
        result = p;
      }
    }
    return result;
  }

  inline constexpr std::optional<int>
  find_coord(std::pair<int8_t, int8_t> coord) const noexcept {
    for (int i = 0; i < N; ++i) {
      if (xy_cords[i] == coord) {
        return i;
      }
    }
    return std::nullopt;
  }

  inline constexpr bool
  has_coord(std::pair<int8_t, int8_t> coord) const noexcept {
    return std::find(xy_cords.begin(), xy_cords.end(), coord) != xy_cords.end();
  }

  inline constexpr std::pair<int8_t, int8_t> max_xy() const noexcept {
    int8_t x_max = xy_cords[0].first;
    int8_t y_max = xy_cords[0].second;
    for (int i = 1; i < N; ++i) {
      x_max = std::max(x_max, xy_cords[i].first);
      y_max = std::max(y_max, xy_cords[i].second);
    }
    return {x_max, y_max};
  }

  void print() const noexcept {
    auto s = _align_to_positive_quadrant();
    std::set<std::pair<int8_t, int8_t>> coord_set(s.xy_cords.begin(),
                                                  s.xy_cords.end());
    const auto [x_max, y_max] = s.max_xy();
    for (int y = 0; y <= y_max; ++y) {
      for (int x = 0; x <= x_max; ++x) {
        if (coord_set.count({x, y})) {
          std::cout << "█";
        } else {
          std::cout << "░";
        }
      }
      std::cout << "\n";
    }
    std::cout << "\n";
  }

  template <typename IT>
  void generate_neighbours(IT storage_iterator) const noexcept {
    static constexpr std::array<std::pair<int8_t, int8_t>, 4> directions = {
        std::pair<int8_t, int8_t>{-1, 0}, std::pair<int8_t, int8_t>{1, 0},
        std::pair<int8_t, int8_t>{0, -1}, std::pair<int8_t, int8_t>{0, 1}};
    std::array<std::pair<int8_t, int8_t>, N + 1> new_coords;
    for (int i = 0; i < N; ++i) {
      new_coords[i] = xy_cords[i];
    }
    std::array<std::pair<int8_t, int8_t>, 3 * N + 1> considered_candidates;
    auto *cur_candidate = considered_candidates.begin();
    for (const auto &[x, y] : xy_cords) {
      for (const auto &[dx, dy] : directions) {
        *cur_candidate = {x + dx, y + dy};
        if (has_coord(*cur_candidate)) {
          continue;
        }
        if (std::find(considered_candidates.begin(), cur_candidate,
                      *cur_candidate) != cur_candidate) {
          continue;
        }
        new_coords[N] = *cur_candidate;
        *storage_iterator = Polyomino<N + 1>{new_coords}.canonical();
        ++storage_iterator;
        ++cur_candidate;
      }
    }
  }

  inline constexpr auto operator<=>(const Polyomino<N> &) const = default;
  friend std::hash<Polyomino<N>>;
};

// Custom specialization of std::hash can be injected in namespace std.
template <int N> struct std::hash<Polyomino<N>> {
  inline std::size_t operator()(const Polyomino<N> &s) const noexcept {
    char buffer[2 * N];
    std::memcpy(buffer, s.xy_cords.data(), 2 * N);
    return std::hash<std::string_view>{}(std::string_view(buffer, 2 * N));
  }
};

template <int N>
std::vector<Polyomino<N + 1>>
get_next_gen(const std::vector<Polyomino<N>> &shapes) noexcept {
  std::vector<Polyomino<N + 1>> result;
  Polyomino<N + 1> invalidVal;
  for (auto &v : invalidVal.xy_cords) {
    v.first = std::numeric_limits<int8_t>::max();
    v.second = std::numeric_limits<int8_t>::max();
  }
  result.resize(shapes.size() * (3 * N + 1), invalidVal);
  const auto start_it = &shapes[0];
  std::for_each(std::execution::par_unseq, shapes.begin(), shapes.end(),
                [&result, start_it](const Polyomino<N> &s) {
                  int idx = &s - start_it;
                  s.generate_neighbours(&result[idx * (3 * N + 1)]);
                });
  result.erase(std::remove_if(result.begin(), result.end(),
                              [&invalidVal](const Polyomino<N + 1> &p) {
                                return p == invalidVal;
                              }),
               result.end());

  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

template <int NUM, int N>
void print_count(std::vector<Polyomino<N>> &&start) noexcept {
  std::cout << "Number of " << N << "-ominoes: " << start.size() << std::endl;
  if constexpr (NUM == 1) {
    return;
  } else {
    print_count<NUM - 1, N + 1>(get_next_gen(start));
  }
}

template <typename T> struct IsPolyomino : std::false_type {};
template <int N> struct IsPolyomino<Polyomino<N>> : std::true_type {};

template <typename T>
concept PolyominoConcept = IsPolyomino<std::remove_cvref_t<T>>::value;

template <int N, int K>
__attribute__((always_inline)) inline constexpr int64_t
TileFitBitMask(const Polyomino<N> &board, const Polyomino<K> &tile, int8_t dx,
               int8_t dy) noexcept {
  static_assert(N > K);
  int64_t result = 0;
  int num_matches = 0;
  int64_t tile_mask = 1;

  for (const auto &xy : tile.xy_cords) {
    if (!board.has_coord({xy.first + dx, xy.second + dy})) {
      return 0;
    }
  }

  for (const auto &xy : board.xy_cords) {
    if (tile.has_coord({xy.first - dx, xy.second - dy})) {
      result |= tile_mask;
      ++num_matches;
    }
    tile_mask <<= 1;
  }
  return (num_matches == K) ? result : 0;
}

template <int N, int K>
inline std::vector<int64_t>
FindMatchPatterns(const Polyomino<N> &board,
                  const Polyomino<K> &tile) noexcept {
  static_assert(N > K);
  static auto matchTimer = TimingLogger::instance().getTimer("FindMatchPatterns");
  matchTimer.tic();
  std::vector<int64_t> tile_masks;
  const auto [x_max, y_max] = board.max_xy();
  for (const auto t : tile.symmetries()) {
    for (int8_t dx = 0; dx <= x_max; ++dx) {
      for (int8_t dy = 0; dy <= y_max; ++dy) {
        auto mask = TileFitBitMask(board, t, dx, dy);
        if (mask != 0) {
          tile_masks.push_back(mask);
        }
      }
    }
  }
  std::sort(tile_masks.begin(), tile_masks.end());
  tile_masks.erase(std::unique(tile_masks.begin(), tile_masks.end()),
                   tile_masks.end());
  matchTimer.toc();
  return tile_masks;
}

template <int N>
inline std::vector<int64_t>
FindMatchPatterns([[maybe_unused]] const Polyomino<N> &board,
                  [[maybe_unused]] const Polyomino<1> &tile) noexcept {
  return {0};
}