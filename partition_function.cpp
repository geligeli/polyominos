#include "partition_function.hpp"

std::vector<std::vector<int>> generate_partitions(int N) {
  std::vector<std::vector<int>> result;
  std::vector<int> current_partition(N, 0);

  auto generate_helper = [&](auto &self, int n, int max_val, int pos) -> void {
    if (n == 0) {
      result.emplace_back(current_partition.begin(), current_partition.begin() + pos);
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