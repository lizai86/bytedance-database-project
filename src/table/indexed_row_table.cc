#include "indexed_row_table.h"
#include <cstring>
#include <iostream>

namespace bytedance_db_project {
IndexedRowTable::IndexedRowTable(int32_t index_column) {
  index_column_ = index_column;
}

void IndexedRowTable::Load(BaseDataLoader *loader) {
    num_cols_ = loader->GetNumCols();
    auto rows = loader->GetRows();
    num_rows_ = rows.size();
    rows_ = new char[FIXED_FIELD_LEN * num_rows_ * num_cols_];
    for (auto row_id = 0; row_id < num_rows_; row_id++) {
        auto cur_row = rows.at(row_id);
        std::memcpy(rows_ + row_id * (FIXED_FIELD_LEN * num_cols_), cur_row,
            FIXED_FIELD_LEN * num_cols_);

        int32_t key;
        char* pointer = rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + index_column_);
        std::memcpy(&key, pointer, FIXED_FIELD_LEN);
        index_.insert(pointer, key);
        
    }
}

int32_t IndexedRowTable::GetIntField(int32_t row_id, int32_t col_id) {
    int32_t ans;
    std::memcpy(&ans, rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), FIXED_FIELD_LEN);
    return ans;
}

void IndexedRowTable::PutIntField(int32_t row_id, int32_t col_id,
                                  int32_t field) {
    //需要更新索引    删除结点    插入结点
    char* pointer = rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id);
    if (col_id == index_column_) {
        int32_t key;
        std::memcpy(&key, pointer, FIXED_FIELD_LEN);
        index_.deleteNode(pointer, key);
        index_.insert(pointer, field);
    }
    std::memcpy(pointer, &field, FIXED_FIELD_LEN);
}

int64_t IndexedRowTable::ColumnSum() {
    int64_t ans = 0;
    if (index_column_ == 0) {
        Bplustree::LeafNode<char, int>* head = index_.getHead();
        while (head != nullptr) {
            for (int i = 0; i < head->numKids; i++) {
                ans += head->keys[i];
            }
            head = head->right;
        }
    } else {
        for (int i = 0; i < num_rows_; i++) {
            ans += GetIntField(i, 0);
        }
    }
    return ans;
}

//SELECT SUM(col0) FROM table WHERE col1 > threshold1 AND col2 < threshold2;
int64_t IndexedRowTable::PredicatedColumnSum(int32_t threshold1,
                                             int32_t threshold2) {
    int64_t ans = 0;
    if (index_column_ == 1) {
        Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_.findLeafNode(threshold1));
        int32_t col0;
        int32_t col2;
        while (leafNode != nullptr) {
            for (int i = 0; i < leafNode->numKids; i++) {
                if (leafNode->keys[i] > threshold1) {
                    std::memcpy(&col2, leafNode->values[i] + FIXED_FIELD_LEN, FIXED_FIELD_LEN);
                    if (col2 < threshold2) {
                        std::memcpy(&col0, leafNode->values[i] - FIXED_FIELD_LEN, FIXED_FIELD_LEN);
                        ans += col0;
                    }
                }
            }
            leafNode = leafNode->right;
        }
    } else if (index_column_ == 2) {
        Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_.findLeafNode(threshold2));
        int32_t col0;
        int32_t col1;
        while (leafNode != nullptr) {
            for (int i = 0; i < leafNode->numKids; i++) {
                if (leafNode->keys[i] < threshold2) {
                    std::memcpy(&col1, leafNode->values[i] - FIXED_FIELD_LEN, FIXED_FIELD_LEN);
                    if (col1 > threshold1) {
                        std::memcpy(&col0, leafNode->values[i] - 2 * FIXED_FIELD_LEN, FIXED_FIELD_LEN);
                        ans += col0;
                    }
                } else {
                    break;
                }
            }
            leafNode = leafNode->left;
        }
    } else {
        int32_t cols[3];
        for (int row_id = 0; row_id < num_rows_; row_id++) {
            std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, 3 * FIXED_FIELD_LEN);
            if (cols[1] > threshold1 && cols[2] < threshold2) {
                ans += cols[0];
            }
        }
    }
    return ans;
}

//SELECT SUM(col0) + ... + SUM(coln) FROM table WHERE col0 > threshold;
int64_t IndexedRowTable::PredicatedAllColumnsSum(int32_t threshold) {
    int64_t ans = 0;
    int32_t* cols = new int32_t[num_cols_];
    if (index_column_ == 0) {
        Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_.findLeafNode(threshold));

        while (leafNode != nullptr) {
            for (int i = 0; i < leafNode->numKids; i++) {
                if (leafNode->keys[i] > threshold) {
                    std::memcpy(cols, leafNode->values[i], num_cols_ * FIXED_FIELD_LEN);
                    for (int col = 0; col < num_cols_; col++) {
                        ans += cols[col];
                    }
                }
            }
            leafNode = leafNode->right;
        }
    } else {
        int32_t* cols = new int32_t[num_cols_];
        for (int row_id = 0; row_id < num_rows_; row_id++) {
            std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, num_cols_ * FIXED_FIELD_LEN);
            if (cols[0] > threshold) {
                for (int col_id = 0; col_id < num_cols_; col_id++) {
                    ans += cols[col_id];
                }
            }
        }
    }
    delete[] cols;
    return ans;
}

//UPDATE table SET col3 = col3 + col2 WHERE col0 < threshold;
//返回更新的行数，即使col2为0，col3值未改变，但也要算作更新（以WHERE条件为准）
int64_t IndexedRowTable::PredicatedUpdate(int32_t threshold) {
    int64_t cnt = 0;
    if (index_column_ == 0) {
        Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_.findLeafNode(threshold));
        int32_t col3;
        int32_t col2;
        while (leafNode != nullptr) {
            for (int i = 0; i < leafNode->numKids; i++) {
                if (leafNode->keys[i] < threshold) {
                    char* pointer = leafNode->values[i] + 3 * FIXED_FIELD_LEN;
                    std::memcpy(&col2, leafNode->values[i] + 2 * FIXED_FIELD_LEN, FIXED_FIELD_LEN);
                    std::memcpy(&col3, pointer, FIXED_FIELD_LEN);
                    col3 += col2;
                    std::memcpy(pointer, &col3, FIXED_FIELD_LEN);
                    cnt++;
                } else {
                    break;
                }
            }
            leafNode = leafNode->left;
        }
    } else {
        int32_t cols[4];
        for (int row_id = 0; row_id < num_rows_; row_id++) {
            std::memcpy(cols, rows_ + FIXED_FIELD_LEN * row_id * num_cols_, 4 * FIXED_FIELD_LEN);
            if (cols[0] < threshold) {
                cnt++;
                int32_t newValue = cols[3] + cols[2];
                PutIntField(row_id, 3, newValue);
            }
        }
    }
    return cnt;
}
} // namespace bytedance_db_project
