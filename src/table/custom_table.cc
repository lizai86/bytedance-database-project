#include "custom_table.h"
#include <cstring>
#include <iostream>

namespace bytedance_db_project {
CustomTable::CustomTable() {}

CustomTable::~CustomTable() {
    if (rows_ != nullptr) {
        delete rows_;
        rows_ = nullptr;
    }
}

void CustomTable::Load(BaseDataLoader *loader) {
    num_cols_ = loader->GetNumCols();
    auto rows = loader->GetRows();
    num_rows_ = rows.size();

    int oneRowLen = FIXED_FIELD_LEN * num_cols_;
    rows_ = new char[oneRowLen * num_rows_];
    char* pointer = rows_;

    for (auto row_id = 0; row_id < num_rows_; row_id++) {
        auto cur_row = rows.at(row_id);
        std::memcpy(pointer, cur_row, oneRowLen);

        int32_t key;
        
        std::memcpy(&key, pointer, FIXED_FIELD_LEN);
        index_0.insert(pointer, key);

        //pointer = rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + 1);
        std::memcpy(&key, pointer + FIXED_FIELD_LEN, FIXED_FIELD_LEN);
        index_1.insert(pointer + FIXED_FIELD_LEN, key);

        pointer += oneRowLen;
    }
}

int32_t CustomTable::GetIntField(int32_t row_id, int32_t col_id) {
    int32_t ans;
    std::memcpy(&ans, rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id), FIXED_FIELD_LEN);
    return ans;
}

void CustomTable::PutIntField(int32_t row_id, int32_t col_id, int32_t field) {
    //需要更新索引    删除结点    插入结点
    char* pointer = rows_ + FIXED_FIELD_LEN * (row_id * num_cols_ + col_id);
    if (col_id == 0) {
        int32_t key;
        std::memcpy(&key, pointer, FIXED_FIELD_LEN);
        index_0.deleteNode(pointer, key);
        std::memcpy(pointer, &field, FIXED_FIELD_LEN);
        index_0.insert(pointer, field);
    } else if (col_id == 1) {
        int32_t key;
        std::memcpy(&key, pointer, FIXED_FIELD_LEN);
        index_1.deleteNode(pointer, key);
        std::memcpy(pointer, &field, FIXED_FIELD_LEN);
        index_1.insert(pointer, field);
    } else {
        std::memcpy(pointer, &field, FIXED_FIELD_LEN);
    }
    
}

int64_t CustomTable::ColumnSum() {
    int64_t ans = 0;
    Bplustree::LeafNode<char, int>* head = index_0.getHead();
    while (head != nullptr) {
        for (int i = 0; i < head->numKids; i++) {
            ans += head->keys[i];
        }
        head = head->right;
    }
    return ans;
}

int64_t CustomTable::PredicatedColumnSum(int32_t threshold1,
                                         int32_t threshold2) {
    int64_t ans = 0;
    Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_1.findLeafNode(threshold1));
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
    return ans;
}

int64_t CustomTable::PredicatedAllColumnsSum(int32_t threshold) {
    int64_t ans = 0;
    int32_t* cols = new int32_t[num_cols_];
    Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_0.findLeafNode(threshold));

    int oneRowLen = num_cols_ * FIXED_FIELD_LEN;
    while (leafNode != nullptr) {
        for (int i = 0; i < leafNode->numKids; i++) {
            if (leafNode->keys[i] > threshold) {
                std::memcpy(cols, leafNode->values[i], oneRowLen);
                for (int col = 0; col < num_cols_; col++) {
                    ans += cols[col];
                }
            }
        }
        leafNode = leafNode->right;
    }

    delete[] cols;
    return ans;
}

int64_t CustomTable::PredicatedUpdate(int32_t threshold) {
    int64_t cnt = 0;
    Bplustree::LeafNode<char, int32_t>* leafNode = static_cast<Bplustree::LeafNode<char, int32_t>*>(index_0.findLeafNode(threshold));
    int32_t col3;
    int32_t col2;
    int twoFieldLen = FIXED_FIELD_LEN + FIXED_FIELD_LEN;
    int threeFieldLen = twoFieldLen + FIXED_FIELD_LEN;
    while (leafNode != nullptr) {
        for (int i = 0; i < leafNode->numKids; i++) {
            if (leafNode->keys[i] < threshold) {
                //char* pointer = leafNode->values[i] + 3 * FIXED_FIELD_LEN;
                char* pointer = leafNode->values[i] + threeFieldLen;
                std::memcpy(&col2, leafNode->values[i] + twoFieldLen, FIXED_FIELD_LEN);
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
  return cnt;
}
} // namespace bytedance_db_project
