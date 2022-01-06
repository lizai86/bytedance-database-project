#include "row_table.h"
#include <cstring>

namespace bytedance_db_project {
RowTable::RowTable() {}

RowTable::~RowTable() {
  if (rows_ != nullptr) {
    delete rows_;
    rows_ = nullptr;
  }
}

void RowTable::Load(BaseDataLoader *loader) {
  num_cols_ = loader->GetNumCols();
  auto rows = loader->GetRows();
  num_rows_ = rows.size();
  rows_ = new char[FIXED_FIELD_LEN * num_rows_ * num_cols_];
  for (auto row_id = 0; row_id < num_rows_; row_id++) {
    auto cur_row = rows.at(row_id);
    std::memcpy(rows_ + row_id * (FIXED_FIELD_LEN * num_cols_), cur_row,
                FIXED_FIELD_LEN * num_cols_);
  }
}

int32_t RowTable::GetIntField(int32_t row_id, int32_t col_id) {
  int32_t ans;
  memcpy(&ans, rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), FIXED_FIELD_LEN);
  return ans;
}

void RowTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
  memcpy(rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), &field, FIXED_FIELD_LEN);
}

int64_t RowTable::ColumnSum() {
  int64_t ans = 0;
  for (int i = 0; i < num_rows_; i++) {
    ans += GetIntField(i, 0);
  }
  return ans;
}

int64_t RowTable::PredicatedColumnSum(int32_t threshold1, int32_t threshold2) {
  // TODO: Implement this!
  return 0;
}

int64_t RowTable::PredicatedAllColumnsSum(int32_t threshold) {
  // TODO: Implement this!
  return 0;
}

int64_t RowTable::PredicatedUpdate(int32_t threshold) {
  // TODO: Implement this!
  return 0;
}
} // namespace bytedance_db_project
