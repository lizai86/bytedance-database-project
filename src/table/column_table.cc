#include "column_table.h"
#include <cstring>
#include <iostream>

namespace bytedance_db_project {
ColumnTable::ColumnTable() {}

ColumnTable::~ColumnTable() {
  if (columns_ != nullptr) {
    delete columns_;
    columns_ = nullptr;
  }
}

//
// columnTable, which stores data in column-major format.
// That is, data is laid out like
//   col 1 | col 2 | ... | col m.
//
void ColumnTable::Load(BaseDataLoader *loader) {
  num_cols_ = loader->GetNumCols();
  std::vector<char *> rows = loader->GetRows();
  num_rows_ = rows.size();
  columns_ = new char[FIXED_FIELD_LEN * num_rows_ * num_cols_];

  for (int32_t row_id = 0; row_id < num_rows_; row_id++) {
    auto cur_row = rows.at(row_id);
    for (int32_t col_id = 0; col_id < num_cols_; col_id++) {
      int32_t offset = FIXED_FIELD_LEN * ((col_id * num_rows_) + row_id);
      std::memcpy(columns_ + offset, cur_row + FIXED_FIELD_LEN * col_id,
                  FIXED_FIELD_LEN);
    }
  }
}

int32_t ColumnTable::GetIntField(int32_t row_id, int32_t col_id) {
    int32_t ans;
    std::memcpy(&ans, columns_ + FIXED_FIELD_LEN * (col_id * num_rows_ + row_id), FIXED_FIELD_LEN);
    return ans;
}

void ColumnTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
    std::memcpy(columns_ + FIXED_FIELD_LEN * (col_id * num_rows_ + row_id), &field, FIXED_FIELD_LEN);
}

int64_t ColumnTable::ColumnSum() {
    int64_t ans = 0;
    for (int i = 0; i < num_rows_; i++) {
        ans += GetIntField(i, 0);
    }
    return ans;
}

//SELECT SUM(col0) FROM table WHERE col1 > threshold1 AND col2 < threshold2;
int64_t ColumnTable::PredicatedColumnSum(int32_t threshold1, int32_t threshold2) {
    int64_t ans = 0;
    int32_t* col0 = new int32_t[num_rows_];
    int32_t* col1 = new int32_t[num_rows_];
    int32_t* col2 = new int32_t[num_rows_];
    int maxLen = num_rows_ * FIXED_FIELD_LEN;
    std::memcpy(col0, columns_, maxLen);
    int offset = maxLen;
    std::memcpy(col1, columns_ + offset, maxLen);
    offset += maxLen;
    std::memcpy(col2, columns_ + offset, maxLen);
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        if (col1[row_id] > threshold1 && col2[row_id] < threshold2) {
            ans += col0[row_id];
        }
    }
    delete[] col0;
    delete[] col1;
    delete[] col2;
    return ans;
}

//SELECT SUM(col0) + ... + SUM(coln) FROM table WHERE col0 > threshold;
int64_t ColumnTable::PredicatedAllColumnsSum(int32_t threshold) {
    int64_t ans = 0;
    int32_t* col0 = new int32_t[num_rows_];
    std::memcpy(col0, columns_, num_rows_ * FIXED_FIELD_LEN);
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        if (col0[row_id] > threshold) {
            ans += col0[row_id];
            for (int col_id = 1; col_id < num_cols_; col_id++) {
                ans += GetIntField(row_id, col_id);
            }
        }
    }
    delete[] col0;
    return ans;
}

//UPDATE table SET col3 = col3 + col2 WHERE col0 < threshold;
//返回更新的行数，即使col2为0，col3值未改变，但也要算作更新（以WHERE条件为准）
int64_t ColumnTable::PredicatedUpdate(int32_t threshold) {
    int64_t cnt = 0;
    int32_t* col0 = new int32_t[num_rows_];
    std::memcpy(col0, columns_, num_rows_ * FIXED_FIELD_LEN);
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        if (col0[row_id] < threshold) {
            cnt++;
            int32_t col3 = GetIntField(row_id, 3);
            int32_t col2 = GetIntField(row_id, 2);
            PutIntField(row_id, 3, col3 + col2);
        }
    }
    delete[] col0;
    return cnt;
}
} // namespace bytedance_db_project