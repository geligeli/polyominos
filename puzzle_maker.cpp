#include <algorithm>
#include <array>
#include <atomic>
#include <bitset>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <execution>
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

std::string HumanReadableTime(int64_t ns) {
  // round the the most significant unit
  if (ns < 1000) {
    return std::to_string(ns) + "ns";
  }
  ns /= 1000;

  if (ns < 1000) {
    return std::to_string(ns) + "us";
  }
  ns /= 1000;

  if (ns < 1000) {
    return std::to_string(ns) + "ms";
  }
  ns /= 1000;
  if (ns < 60) {
    return std::to_string(ns) + "s";
  }
  ns /= 60;
  return std::to_string(ns) + "m";
}

class TimingLogger {
 public:
  class Timer {
   public:
    void tic() { m_start = std::chrono::high_resolution_clock::now(); }
    void toc() {
      m_ns_timer->fetch_add(
          std::chrono::duration_cast<std::chrono::nanoseconds>(
              std::chrono::high_resolution_clock::now() - m_start)
              .count());
    }
    Timer(std::atomic<int64_t>& ns_timer) : m_ns_timer(&ns_timer) {}

   private:
    std::chrono::high_resolution_clock::time_point m_start;
    std::atomic<int64_t>* m_ns_timer;
  };

  Timer getTimer(const std::string& name) {
    std::unique_lock lk(m_mutex);
    return Timer(m_ns_timers[name]);
  }

  TimingLogger() {
    // m_loggingThread = std::jthread([this](std::stop_token st) {
    //     while (!st.stop_requested()) {
    //         std::unique_lock lk(m_mutex);
    //         m_cv.wait_for(lk, std::chrono::seconds(10), [&st](){return
    //         st.stop_requested();}); for (const auto& [name, timer] :
    //         m_ns_timers) {
    //             std::cout << std::left << std::setw(30) << name <<
    //             HumanReadableTime(timer.load()) << std::endl;
    //         }
    //     }
    // });
  }

  ~TimingLogger() {
    m_loggingThread.request_stop();
    m_cv.notify_all();
  }

 private:
  std::condition_variable m_cv;
  std::mutex m_mutex;
  std::unordered_map<std::string, std::atomic<int64_t>> m_ns_timers;
  std::jthread m_loggingThread;
};

static TimingLogger g_timingLogger;

static constexpr std::array<uint64_t, 64> kPartitionNumber = {
    1ull,        // p(0)
    1ull,        // p(1)
    2ull,        // p(2)
    3ull,        // p(3)
    5ull,        // p(4)
    7ull,        // p(5)
    11ull,       // p(6)
    15ull,       // p(7)
    22ull,       // p(8)
    30ull,       // p(9)
    42ull,       // p(10)
    56ull,       // p(11)
    77ull,       // p(12)
    101ull,      // p(13)
    135ull,      // p(14)
    176ull,      // p(15)
    231ull,      // p(16)
    297ull,      // p(17)
    385ull,      // p(18)
    490ull,      // p(19)
    627ull,      // p(20)
    792ull,      // p(21)
    1002ull,     // p(22)
    1255ull,     // p(23)
    1575ull,     // p(24)
    1958ull,     // p(25)
    2436ull,     // p(26)
    3010ull,     // p(27)
    3718ull,     // p(28)
    4565ull,     // p(29)
    5604ull,     // p(30)
    6842ull,     // p(31)
    8349ull,     // p(32)
    10143ull,    // p(33)
    12310ull,    // p(34)
    14883ull,    // p(35)
    17977ull,    // p(36)
    21637ull,    // p(37)
    26015ull,    // p(38)
    31185ull,    // p(39)
    37338ull,    // p(40)
    44583ull,    // p(41)
    53174ull,    // p(42)
    63261ull,    // p(43)
    75175ull,    // p(44)
    89134ull,    // p(45)
    105558ull,   // p(46)
    124754ull,   // p(47)
    147273ull,   // p(48)
    173525ull,   // p(49)
    204226ull,   // p(50)
    239943ull,   // p(51)
    281589ull,   // p(52)
    329931ull,   // p(53)
    386155ull,   // p(54)
    451276ull,   // p(55)
    526823ull,   // p(56)
    614154ull,   // p(57)
    715220ull,   // p(58)
    831820ull,   // p(59)
    966467ull,   // p(60)
    1121505ull,  // p(61)
    1300156ull,  // p(62)
    1505499ull   // p(63)
};

template <int N>
static constexpr auto generate_partitions() {
  constexpr int size = kPartitionNumber[N];
  std::array<std::array<int, N>, size> result = {};
  int current_partition[N] = {0};
  int partition_index = 0;

  auto generate_helper = [&](auto& self, int n, int max_val, int pos) -> void {
    if (n == 0) {
      std::array<int, N> partition = {};
      for (int i = 0; i < pos; ++i) {
        partition[i] = current_partition[i];
      }
      result[partition_index++] = partition;
      return;
    }

    for (int i = std::min(max_val, n); i >= 1; --i) {
      current_partition[pos] = i;
      self(self, n - i, i, pos + 1);
    }
  };

  generate_helper(generate_helper, N, N, 0);
  return result;
}

template <int N>
struct Polyomino {
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

  inline constexpr std::optional<int> find_coord(
      std::pair<int8_t, int8_t> coord) const noexcept {
    for (int i = 0; i < N; ++i) {
      if (xy_cords[i] == coord) {
        return i;
      }
    }
    return std::nullopt;
  }

  inline constexpr bool has_coord(
      std::pair<int8_t, int8_t> coord) const noexcept {
    for (int i = 0; i < N; ++i) {
      if (xy_cords[i] == coord) {
        return true;
      }
    }
    return false;
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
    auto* cur_candidate = considered_candidates.begin();
    for (const auto& [x, y] : xy_cords) {
      for (const auto& [dx, dy] : directions) {
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

  inline constexpr auto operator<=>(const Polyomino<N>&) const = default;
  friend std::hash<Polyomino<N>>;
};

// Custom specialization of std::hash can be injected in namespace std.
template <int N>
struct std::hash<Polyomino<N>> {
  inline std::size_t operator()(const Polyomino<N>& s) const noexcept {
    char buffer[2 * N];
    std::memcpy(buffer, s.xy_cords.data(), 2 * N);
    return std::hash<std::string_view>{}(std::string_view(buffer, 2 * N));
  }
};

template <int N>
std::vector<Polyomino<N + 1>> get_next_gen(
    const std::vector<Polyomino<N>>& shapes) noexcept {
  std::vector<Polyomino<N + 1>> result;
  Polyomino<N + 1> invalidVal;
  for (auto& v : invalidVal.xy_cords) {
    v.first = std::numeric_limits<int8_t>::max();
    v.second = std::numeric_limits<int8_t>::max();
  }
  result.resize(shapes.size() * (3 * N + 1), invalidVal);
  const auto start_it = &shapes[0];
  std::for_each(std::execution::par_unseq, shapes.begin(), shapes.end(),
                [&result, start_it](const Polyomino<N>& s) {
                  int idx = &s - start_it;
                  s.generate_neighbours(&result[idx * (3 * N + 1)]);
                });
  result.erase(std::remove_if(result.begin(), result.end(),
                              [&invalidVal](const Polyomino<N + 1>& p) {
                                return p == invalidVal;
                              }),
               result.end());

  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  return result;
}

template <int NUM, int N>
void print_count(std::vector<Polyomino<N>>&& start) noexcept {
  std::cout << "Number of " << N << "-ominoes: " << start.size() << std::endl;
  if constexpr (NUM == 1) {
    return;
  } else {
    print_count<NUM - 1, N + 1>(get_next_gen(start));
  }
}

template <typename T>
struct IsPolyomino : std::false_type {};
template <int N>
struct IsPolyomino<Polyomino<N>> : std::true_type {};

template <typename T>
concept PolyominoConcept = IsPolyomino<std::remove_cvref_t<T>>::value;

template <int N, int K>
inline constexpr int64_t TileFitBitMask(const Polyomino<N>& board,
                                        const Polyomino<K>& tile) noexcept {
  static_assert(N > K);
  int64_t result = 0;
  int num_matches = 0;
  int64_t tile_mask = 1;

  for (const auto& xy : board.xy_cords) {
    if (tile.has_coord(xy)) {
      result |= tile_mask;
      ++num_matches;
    }
    tile_mask <<= 1;
  }
  return (num_matches == K) ? result : 0;
}

template <int N, int K>
inline std::vector<int64_t> FindMatchPatterns(
    const Polyomino<N>& board, const Polyomino<K>& tile) noexcept {
  static_assert(N > K);
  static auto matchTimer = g_timingLogger.getTimer("FindMatchPatterns");
  matchTimer.tic();
  std::vector<int64_t> tile_masks;
  for (const auto t : tile.symmetries()) {
    for (int8_t dx = 0; dx < N; ++dx) {
      for (int8_t dy = 0; dy < N; ++dy) {
        Polyomino<K> translated_tile = t.translate_xy(dx, dy);
        auto mask = TileFitBitMask(board, translated_tile);
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
inline std::vector<int64_t> FindMatchPatterns(
    [[maybe_unused]] const Polyomino<N>& board,
    [[maybe_unused]] const Polyomino<1>& tile) noexcept {
  return {0};
}

// template <int N>
struct SolutionStats {
  int64_t possibilities_at_last_step{};
  int64_t solution_count{};

  inline constexpr int64_t last_step_difficulty() const {
    return possibilities_at_last_step - solution_count;
  }

  inline constexpr std::weak_ordering operator<=>(
      const SolutionStats& other) const {
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

template <int N, PolyominoConcept... Tiles>
struct PuzzleParams {
  static constexpr auto kNumTiles = sizeof...(Tiles);
  std::array<std::vector<int64_t>, sizeof...(Tiles)> params;
  std::tuple<Tiles...> tiles;
  Polyomino<N> board;

  template <PolyominoConcept NextTile>
  constexpr std::optional<PuzzleParams<N, NextTile, Tiles...>> operator()(
      NextTile next_tile) const {
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
    std::apply([](auto&&... args) { ((args.print()), ...); }, tiles);
  }
};

template <int N, PolyominoConcept... Tiles>
class PuzzleSolver {
 public:
  SolutionStats& stats;
  const PuzzleParams<N, Tiles...>& puzzle_params;
  std::array<int, sizeof...(Tiles)> indices;
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
          const auto& tile = puzzle_params.params[i][indices[i]];
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
};

template <int N>
struct PuzzleBoard {
  Polyomino<N> board;
  template <PolyominoConcept NextTile>
  constexpr std::optional<PuzzleParams<N, NextTile>> operator()(
      NextTile next_tile) const {
    auto next_param = FindMatchPatterns(board, next_tile);
    if (next_param.empty()) {
      return std::nullopt;
    }
    return PuzzleParams<N, NextTile>{
        std::array<std::vector<int64_t>, 1>{next_param},
        std::tuple<NextTile>(next_tile), board};
  }
};

std::vector<int64_t> CrossProduct(const std::vector<int64_t>& a,
                                  const std::vector<int64_t>& b) {
  std::vector<int64_t> result;
  for (const auto& x : a) {
    for (const auto& y : b) {
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
    PuzzleParams<N, Tiles...>& puzzle_configuration) noexcept {
  // template <int N, PolyominoConcept... Tiles>
  // SolutionStats SolvePuzzle(
  //     PuzzleParams<N, Tiles...>& puzzle_configuration) noexcept {
  //   SolutionStats stats;

  auto& params = puzzle_configuration.params;

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
            [](const auto& a, const auto& b) { return a.size() < b.size(); });

  // PuzzleSolver<puzzle_configuration::kNumTiles> solver{stats,
  // puzzle_configuration.params};
}

template <int N, int M>
inline constexpr Polyomino<N * M> CreateRectangle() {
  std::array<std::pair<int8_t, int8_t>, N * M> coords;
  for (int i = 0; i < N; ++i) {
    for (int j = 0; j < M; ++j) {
      coords[i * M + j] = {i, j};
    }
  }
  return Polyomino<N * M>{coords}.canonical();
}

template <int N>
inline constexpr Polyomino<N * N> CreateSquare() {
  return CreateRectangle<N, N>();
}

template <int N>
struct PrecomputedPolyminosSet {
  static const auto& polyminos() {
    static const auto val = []() {
      auto result = get_next_gen(PrecomputedPolyminosSet<N - 1>::polyminos());
      std::cout << "Generated polyminos for N=" << N << std::endl;
      return result;
    }();
    return val;
  }
};

template <>
struct PrecomputedPolyminosSet<1> {
  static const auto& polyminos() {
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
constexpr bool AcceptPartition(const std::array<int, N>& partition) {
  if (partition[0] == N) {
    return false;
  } else if (partition[0] > 10) {
    return false;
  }
  int partition_size = 0;
  int last_p = partition[0];
  int number_of_ones = 0;
  for (auto p : partition) {
    if (p == 0) {
      break;
    }
    if (p == 1) {
      ++number_of_ones;
    }
    ++partition_size;
    last_p = p;
  }
  // return partition_size > 5 && last_p > 2;
  // return partition_size > 3;
  return true;
}

template <int N, std::array<int, N> PARTITION_OF_N, std::size_t CUR_INDEX = 0>
void TryAllPossibilitiesForPartition(const Polyomino<N>& board,
                                     auto&& param_generator,
                                     const auto& puzzle_configuration_visitor) {
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

      for (const auto& p : PrecomputedPolyminosSet<
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
      for (const auto& p : PrecomputedPolyminosSet<
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
    const Polyomino<N>& board, const auto& puzzle_configuration_visitor,
    [[maybe_unused]] std::integer_sequence<int, ints...> ints_sequence) {
  constexpr auto kPartitions = generate_partitions<N>();
  (TryAllPossibilitiesForPartition<N, kPartitions[ints]>(
       board, PuzzleBoard(board), puzzle_configuration_visitor),
   ...);
}

template <int N>
void ForAllPartitions(const Polyomino<N>& board,
                      const auto& puzzle_configuration_visitor) {
  _ForAllPartitions(board, puzzle_configuration_visitor,
                    std::make_integer_sequence<int, kPartitionNumber[N]>());
}

int main() {
  std::mutex m;
  SolutionStats best_stats{};
  std::atomic<int64_t> puzzle_configs = 0;

  auto puzzle_configuration_visitor =
      [&]<int N, PolyominoConcept... Tiles>(
          PuzzleParams<N, Tiles...>&& puzzle_configuration) {
        thread_local static auto puzzleTimer =
            g_timingLogger.getTimer("SolvePuzzle");
        puzzleTimer.tic();

        PreProcessConfiguration(puzzle_configuration);
        SolutionStats stats{};
        PuzzleSolver solver{stats, puzzle_configuration};
        solver.Solve();
        ++puzzle_configs;
        puzzleTimer.toc();
        if (solver.earlyAbort) {
          return;
        }
        std::lock_guard lk(m);
        if (stats > best_stats) {
          best_stats = stats;
          std::cout << "Solutions: " << stats.solution_count << std::endl;

          std::cout << "Possibilities at last step: "
                    << stats.possibilities_at_last_step << std::endl;
          std::cout << "Puzzle: " << std::endl;
          puzzle_configuration.board.print();
          std::apply([](auto&&... args) { ((args.print()), ...); },
                     puzzle_configuration.tiles);
          std::cout << "Solution: \n";
          std::cout << solver.decodeSolution() << "\n";
        }
      };

  const auto visitor = [&](const auto& board) {
    ForAllPartitions(board, puzzle_configuration_visitor);
  };

  const auto& kBoards = PrecomputedPolyminosSet<14>::polyminos();
  std::for_each(std::execution::par_unseq, kBoards.begin(), kBoards.end(),
                visitor);

  std::cout << "Puzzles solved=" << puzzle_configs << std::endl;

  return 0;
}
