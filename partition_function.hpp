#pragma once

#include <array>
#include <cstdint>

static constexpr std::array<uint64_t, 64> kNumberOfPartitions = {
    1ull,       // p(0)
    1ull,       // p(1)
    2ull,       // p(2)
    3ull,       // p(3)
    5ull,       // p(4)
    7ull,       // p(5)
    11ull,      // p(6)
    15ull,      // p(7)
    22ull,      // p(8)
    30ull,      // p(9)
    42ull,      // p(10)
    56ull,      // p(11)
    77ull,      // p(12)
    101ull,     // p(13)
    135ull,     // p(14)
    176ull,     // p(15)
    231ull,     // p(16)
    297ull,     // p(17)
    385ull,     // p(18)
    490ull,     // p(19)
    627ull,     // p(20)
    792ull,     // p(21)
    1002ull,    // p(22)
    1255ull,    // p(23)
    1575ull,    // p(24)
    1958ull,    // p(25)
    2436ull,    // p(26)
    3010ull,    // p(27)
    3718ull,    // p(28)
    4565ull,    // p(29)
    5604ull,    // p(30)
    6842ull,    // p(31)
    8349ull,    // p(32)
    10143ull,   // p(33)
    12310ull,   // p(34)
    14883ull,   // p(35)
    17977ull,   // p(36)
    21637ull,   // p(37)
    26015ull,   // p(38)
    31185ull,   // p(39)
    37338ull,   // p(40)
    44583ull,   // p(41)
    53174ull,   // p(42)
    63261ull,   // p(43)
    75175ull,   // p(44)
    89134ull,   // p(45)
    105558ull,  // p(46)
    124754ull,  // p(47)
    147273ull,  // p(48)
    173525ull,  // p(49)
    204226ull,  // p(50)
    239943ull,  // p(51)
    281589ull,  // p(52)
    329931ull,  // p(53)
    386155ull,  // p(54)
    451276ull,  // p(55)
    526823ull,  // p(56)
    614154ull,  // p(57)
    715220ull,  // p(58)
    831820ull,  // p(59)
    966467ull,  // p(60)
    1121505ull, // p(61)
    1300156ull, // p(62)
    1505499ull  // p(63)
};

template <int N> static constexpr auto generate_partitions() {
  std::array<std::array<int, N>,  kNumberOfPartitions[N]> result = {};
  int current_partition[N] = {0};
  int partition_index = 0;

  auto generate_helper = [&](auto &self, int n, int max_val, int pos) -> void {
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
struct Partition {
  static constexpr auto kPartition = generate_partitions<N>();
};

template <> struct Partition<1> { static constexpr auto kPartition = generate_partitions<1>(); };
template <> struct Partition<2> { static constexpr auto kPartition = generate_partitions<2>(); };
template <> struct Partition<3> { static constexpr auto kPartition = generate_partitions<3>(); };
template <> struct Partition<4> { static constexpr auto kPartition = generate_partitions<4>(); };
template <> struct Partition<5> { static constexpr auto kPartition = generate_partitions<5>(); };
template <> struct Partition<6> { static constexpr auto kPartition = generate_partitions<6>(); };
template <> struct Partition<7> { static constexpr auto kPartition = generate_partitions<7>(); };
template <> struct Partition<8> { static constexpr auto kPartition = generate_partitions<8>(); };
template <> struct Partition<9> { static constexpr auto kPartition = generate_partitions<9>(); };
template <> struct Partition<10> { static constexpr auto kPartition = generate_partitions<10>(); };
template <> struct Partition<11> { static constexpr auto kPartition = generate_partitions<11>(); };
template <> struct Partition<12> { static constexpr auto kPartition = generate_partitions<12>(); };
template <> struct Partition<13> { static constexpr auto kPartition = generate_partitions<13>(); };
template <> struct Partition<14> { static constexpr auto kPartition = generate_partitions<14>(); };
template <> struct Partition<15> { static constexpr auto kPartition = generate_partitions<15>(); };
template <> struct Partition<16> { static constexpr auto kPartition = generate_partitions<16>(); };
