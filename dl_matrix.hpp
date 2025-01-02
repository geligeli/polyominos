#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <limits>

class DLMatrix {
public:
  using PtrType = int16_t; // -1 means nullptr
  using ColumnIndex = int16_t;
  using RowIndex = int16_t;

  // v is a vector of bitmasks
  explicit DLMatrix(const std::vector<uint64_t> &v);

  void CoverColumn(ColumnIndex col_idx);
  void UncoverColum(ColumnIndex col_idx);

  std::string DebugString() const;

  template <typename Func> void VisitAllOnes(Func &&visitor) const {
    WalkAllCols([&, this](ColumnIndex col) {
      visitor(col, -1);
      WalkDownCol(col, [&](PtrType elem) {
        visitor(entries[elem].col, entries[elem].row);
      });
    });
  }

  struct Entry {
    PtrType left;
    PtrType right;
    PtrType up;
    PtrType down;

    ColumnIndex col;
    RowIndex row;
  };

  struct ColHeader {
    ColumnIndex left;
    ColumnIndex right;
    PtrType down;
    PtrType up;

    std::size_t col_size;
  };

  enum class Status {
    OK,
    NO_SOLUTION,
    SOLVED,
  };

  Status status() const;

  std::vector<ColHeader> col_headers;
  std::vector<Entry> entries;

  const ColHeader &root() const;

private:
  template <typename Visitor> void WalkAllCols(Visitor &&visitor) const {
    const auto root_idx = col_headers.size() - 1;
    for (ColumnIndex idx = col_headers[root_idx].right; idx != root_idx;
         idx = col_headers[idx].right) {
      visitor(idx);
    }
  }

  template <typename Visitor>
  void WalkDownCol(ColumnIndex col_idx, Visitor &&visitor) const {
    for (PtrType idx = col_headers[col_idx].down; idx != -1;
         idx = entries[idx].down) {
      if constexpr (std::is_void_v<std::invoke_result_t<Visitor, PtrType>>) {
        visitor(idx);
      } else {
        if (!visitor(idx)) {
          break;
        }
      }
    }
  }
  template <typename Visitor>
  void WalkUpCol(ColumnIndex col_idx, Visitor &&visitor) const {
    for (PtrType idx = col_headers[col_idx].up; idx != -1;
         idx = entries[idx].up) {
      visitor(idx);
    }
  }
  template <typename Visitor>
  void WalkRowRight(PtrType elem, Visitor &&visitor) const {
    for (PtrType idx = entries[elem].right; idx != elem;
         idx = entries[idx].right) {
      visitor(idx);
    }
  }
  template <typename Visitor>
  void WalkRowLeft(PtrType elem, Visitor &&visitor) const {
    for (PtrType idx = entries[elem].left; idx != elem;
         idx = entries[idx].left) {
      visitor(idx);
    }
  }

  void DetachColumnHeader(ColumnIndex col_idx);
  void RestoreColumnHeader(ColumnIndex col_idx);

  void DetachEntryFromColumn(PtrType entry_idx);
  void RestoreEntryFromColumn(PtrType entry_idx);  
  
  // return true if we should continue exploring
  template <typename ON_SOL, typename ON_STUCK, typename ON_TRY, typename ON_UNDO>
  bool Recurse(ON_SOL&& on_sol, ON_STUCK&& on_stuck, ON_TRY&& on_try, ON_UNDO&& on_undo) {
    DLMatrix::ColumnIndex col = root().right;
    if (col == col_headers.size() - 1) {
      return on_sol();
    }
    DLMatrix::ColumnIndex min_col = col_headers.size() - 1;
    std::size_t min_col_size = std::numeric_limits<std::size_t>::max();
    WalkAllCols([&](DLMatrix::ColumnIndex idx) {
      if (col_headers[idx].col_size <
          min_col_size) {
        min_col_size = col_headers[idx].col_size;
        min_col = idx;
      }
    });

    if (min_col_size == 0) {
      on_stuck(min_col);
      // no solution, go on.
      return true;
    }
    CoverColumn(min_col);
    bool keep_going = true;
    WalkDownCol(min_col, [&](DLMatrix::PtrType elem) {
      on_try( entries[elem].row );
      WalkRowRight(elem, [&](DLMatrix::PtrType idx) {
        CoverColumn(entries[idx].col);
      });
      keep_going = Recurse(on_sol, on_stuck, on_try, on_undo);
      on_undo();
      
      WalkRowLeft(elem, [&](DLMatrix::PtrType idx) {
        UncoverColum(entries[idx].col);
      });
      return keep_going;
    });
    return keep_going;
  };


  friend bool SolveCoverProblem(const std::vector<uint64_t> &v,
                                std::vector<std::size_t> &rows);

  friend void
  ExhaustiveSolveCoverProblem(const std::vector<uint64_t> &v,
                              std::vector<std::vector<std::size_t>> &solutions);
};

bool SolveCoverProblem(const std::vector<uint64_t> &v,
                       std::vector<std::size_t> &rows);

void ExhaustiveSolveCoverProblem(
    const std::vector<uint64_t> &v,
    std::vector<std::vector<std::size_t>> &solutions);
