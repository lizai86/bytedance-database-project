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
    std::memcpy(&ans, rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), FIXED_FIELD_LEN);
    return ans;
}

void RowTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
    std::memcpy(rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), &field, FIXED_FIELD_LEN);
}

int64_t RowTable::ColumnSum() {
    int64_t ans = 0;
    for (int i = 0; i < num_rows_; i++) {
        ans += GetIntField(i, 0);
    }
    return ans;
}

//SELECT SUM(col0) FROM table WHERE col1 > threshold1 AND col2 < threshold2;
int64_t RowTable::PredicatedColumnSum(int32_t threshold1, int32_t threshold2) {
    int64_t ans = 0;
    int32_t cols[3];
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, 3 * FIXED_FIELD_LEN);
        if (cols[1] > threshold1 && cols[2] < threshold2) {
            ans += cols[0];
        }
    }
    return ans;
}

//SELECT SUM(col0) + ... + SUM(coln) FROM table WHERE col0 > threshold;
int64_t RowTable::PredicatedAllColumnsSum(int32_t threshold) {
    int64_t ans = 0;
    int32_t* cols = new int32_t[num_cols_];
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, num_cols_ * FIXED_FIELD_LEN);
        if (cols[0] > threshold) {
            for (int col_id = 0; col_id < num_cols_; col_id++) {
                ans += cols[col_id];
            }
        }
    }
    delete[] cols;
    return ans;
}

//UPDATE table SET col3 = col3 + col2 WHERE col0 < threshold;
//返回更新的行数，即使col2为0，col3值未改变，但也要算作更新（以WHERE条件为准）
int64_t RowTable::PredicatedUpdate(int32_t threshold) {
    int64_t cnt = 0;
    int32_t cols[4];
    for (int row_id = 0; row_id < num_rows_; row_id++) {
        std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, 4 * FIXED_FIELD_LEN);
        if (cols[0] < threshold) {
            cnt++;
            int32_t newValue = cols[3] + cols[2];
            PutIntField(row_id, 3, newValue);
        }
    }
    return cnt;
}
} // namespace bytedance_db_project