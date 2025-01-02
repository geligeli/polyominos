#include "dl_matrix.hpp"
#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <unordered_set>

// v is a vector of bitmasks
DLMatrix::DLMatrix(const std::vector<uint64_t> &v) {
  const auto num_entries =
      std::transform_reduce(v.begin(), v.end(), 0u, std::plus<uint64_t>{},
                            [](auto x) { return std::popcount(x); });
  const auto num_columns = std::transform_reduce(
      v.begin(), v.end(), 0u,
      [](uint64_t a, uint64_t b) { return std::max<uint64_t>(a, b); },
      [](auto x) { return static_cast<uint64_t>(std::bit_width(x)); });
  entries.reserve(num_entries);
  col_headers.resize(num_columns + 1);
  for (std::size_t col_idx = 0; col_idx <= num_columns; ++col_idx) {

    col_headers[col_idx].left = col_idx == 0 ? num_columns : col_idx - 1;
    col_headers[col_idx].right = col_idx == num_columns ? 0 : col_idx + 1;
    col_headers[col_idx].col_size = 0;
    col_headers[col_idx].down = -1;
    col_headers[col_idx].up = -1;
  }
  std::vector<PtrType> idx_to_previous_entry_in_col(num_columns, -1);
  for (std::size_t row = 0; row < v.size(); ++row) {
    Entry entry;
    entry.row = row;
    entry.down = -1;
    entry.right = -1;
    PtrType last_entry_in_row = -1;
    PtrType first_entry_in_row = -1;
    for (std::size_t col_idx = 0; col_idx < num_columns; ++col_idx) {
      if (v[row] & (1ull << col_idx)) {
        auto &col_header = col_headers[col_idx];
        ++col_header.col_size;
        const auto cur_index = entries.size();
        entry.col = col_idx;
        entry.up = idx_to_previous_entry_in_col[col_idx];
        entry.left = last_entry_in_row;
        col_header.up = cur_index;
        if (entry.up == -1) {
          col_header.down = cur_index;
        } else {
          entries[entry.up].down = cur_index;
        }
        if (entry.left != -1) {
          entries[entry.left].right = cur_index;
        }
        idx_to_previous_entry_in_col[col_idx] = cur_index;
        last_entry_in_row = cur_index;
        if (first_entry_in_row == -1) {
          first_entry_in_row = cur_index;
        }
        entries.push_back(entry);
      }
    }
    if (first_entry_in_row != -1) {
      entries[first_entry_in_row].left = last_entry_in_row;
      entries[last_entry_in_row].right = first_entry_in_row;
    }
  }
}

const DLMatrix::ColHeader &DLMatrix::root() const { return col_headers.back(); }

void DLMatrix::DetachEntryFromColumn(PtrType entry_idx) {
  const auto &entry = entries[entry_idx];
  auto &col_header = col_headers[entry.col];
  if (entry.up == -1) {
    col_header.down = entry.down;
  } else {
    entries[entry.up].down = entry.down;
  }
  if (entry.down == -1) {
    col_header.up = entry.up;
  } else {
    entries[entry.down].up = entry.up;
  }
  --col_header.col_size;
}

void DLMatrix::RestoreEntryFromColumn(PtrType entry_idx) {
  const auto &entry = entries[entry_idx];
  auto &col_header = col_headers[entry.col];
  if (entry.up == -1) {
    col_header.down = entry_idx;
  } else {
    entries[entry.up].down = entry_idx;
  }
  if (entry.down == -1) {
    col_header.up = entry_idx;
  } else {
    entries[entry.down].up = entry_idx;
  }
  ++col_header.col_size;
}

std::string DLMatrix::DebugString() const {
  std::size_t num_rows = 0;
  std::size_t num_cols = 0;
  std::vector<std::size_t> seen_rows;
  seen_rows.reserve(entries.size());
  std::vector<std::size_t> seen_cols;
  seen_cols.reserve(entries.size());

  VisitAllOnes([&](auto col, auto row) {
    if (row == -1) {
      seen_cols.push_back(col);
      num_cols = std::max<std::size_t>(num_cols, col + 1);
      return;
    }
    seen_rows.push_back(row);
    num_rows = std::max<std::size_t>(num_rows, row + 1);
  });

  std::sort(seen_rows.begin(), seen_rows.end());
  std::sort(seen_cols.begin(), seen_cols.end());
  seen_rows.erase(std::unique(seen_rows.begin(), seen_rows.end()),
                  seen_rows.end());
  seen_cols.erase(std::unique(seen_cols.begin(), seen_cols.end()),
                  seen_cols.end());

  std::vector<std::size_t> row_to_debug_str_row(num_rows);
  std::vector<std::size_t> col_to_debug_str_col(num_cols);
  for (std::size_t debug_str_row = 0; debug_str_row < seen_rows.size();
       ++debug_str_row) {
    row_to_debug_str_row[seen_rows[debug_str_row]] = debug_str_row + 1;
  }

  for (std::size_t debug_str_col = 0; debug_str_col < seen_cols.size();
       ++debug_str_col) {
    col_to_debug_str_col[seen_cols[debug_str_col]] = debug_str_col;
  }
  const auto num_debug_str_cols = seen_cols.size() + 3;
  const auto num_debug_str_rows = seen_rows.size() + 1;
  std::string result(num_debug_str_rows * num_debug_str_cols, '0');
  VisitAllOnes([&](auto col, auto row) {
    if (row == -1) {
      return;
    }
    result[row_to_debug_str_row[row] * (num_debug_str_cols) +
           num_debug_str_cols - col_to_debug_str_col[col] - 2] = '1';
  });
  result[0] = ' ';
  result[1] = ' ';
  static constexpr std::array<char, 16> kHex = {'0', '1', '2', '3', '4', '5',
                                                '6', '7', '8', '9', 'A', 'B',
                                                'C', 'D', 'E', 'F'};

  for (std::size_t i = 0; i < seen_cols.size(); ++i) {
    result[num_debug_str_cols - col_to_debug_str_col[seen_cols[i]] - 2] =
        kHex[seen_cols[i] % 16];
  }
  result[num_debug_str_cols - 1] = '\n';
  for (std::size_t i = 1; i < num_debug_str_rows; ++i) {
    result[i * num_debug_str_cols + num_debug_str_cols - 1] = '\n';
    result[i * num_debug_str_cols + 0] = kHex[seen_rows[i - 1] % 16];
    result[i * num_debug_str_cols + 1] = ' ';
  }
  return result;
}

void DLMatrix::DetachColumnHeader(ColumnIndex col_idx) {
  col_headers[col_headers[col_idx].left].right = col_headers[col_idx].right;
  col_headers[col_headers[col_idx].right].left = col_headers[col_idx].left;
}

void DLMatrix::RestoreColumnHeader(ColumnIndex col_idx) {
  col_headers[col_headers[col_idx].left].right = col_idx;
  col_headers[col_headers[col_idx].right].left = col_idx;
}

void DLMatrix::CoverColumn(ColumnIndex col_idx) {
  DetachColumnHeader(col_idx);
  WalkDownCol(col_idx, [this](PtrType elem) {
    WalkRowRight(elem, [this](PtrType idx) { DetachEntryFromColumn(idx); });
  });
}

void DLMatrix::UncoverColum(ColumnIndex col_idx) {
  WalkUpCol(col_idx, [this](PtrType elem) {
    WalkRowLeft(elem, [this](PtrType idx) { RestoreEntryFromColumn(idx); });
  });
  RestoreColumnHeader(col_idx);
}

bool SolveCoverProblem(const std::vector<uint64_t> &v,
                       std::vector<std::size_t> &solution) {
  DLMatrix dl_matrix(v);
  bool found_solution = false;
  dl_matrix.Recurse(
      [&]() {
        found_solution = true;
        return false;
      },
      [&](DLMatrix::ColumnIndex col) {},
      [&](std::size_t row) { solution.push_back(row); },
      [&]() {
        if (!found_solution) {
          solution.pop_back();
        }
      });
  return found_solution;
}

void ExhaustiveSolveCoverProblem(
    const std::vector<uint64_t> &v,
    std::vector<std::vector<std::size_t>> &solutions) {
  DLMatrix dl_matrix(v);
  std::vector<std::size_t> solution;
  dl_matrix.Recurse(
      [&]() {
        solutions.push_back(solution);
        return true;
      },
      [&](DLMatrix::ColumnIndex col) {},
      [&](std::size_t row) { solution.push_back(row); },
      [&]() { solution.pop_back(); });
}
