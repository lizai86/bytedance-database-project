#pragma once
#include <limits.h>

namespace Bplustree {
	//结点（非叶子结点和叶子结点）
	template <class T, class V>
	class Node {
	public:
		Node(int degree);
		virtual ~Node();
		virtual T* find(V key) = 0;
		virtual Node<T, V>* findLeafNode(V key) = 0;
		virtual Node<T, V>* insert(T* value, V key) = 0;
		virtual Node<T, V>* del(T* value, V key) = 0;
		virtual Node<T, V>* updateHead() = 0;
		
		//父节点
		Node<T, V>* parent;
		//孩子节点
		Node<T, V>** kids;
		//键
		V* keys;
		//孩子节点的个数
		int numKids;
		//结点的度
		int degree;
	private:
	};

	//非叶子结点
	template <class T, class V>
	class BPlusTreeNode : public Node<T, V>
	{
	public:
		BPlusTreeNode(int degree);
		~BPlusTreeNode() = default;

		T* find(V key);
		Node<T, V>* findLeafNode(V key);
		Node<T, V>* insert(T* value, V key);
		Node<T, V>* del(T* value, V key);
		Node<T, V>* updateHead();
		Node<T, V>* insertKid(Node<T, V>* left, Node<T, V>* right, V maxNum);
		Node<T, V>* delKid(Node<T, V>* left, Node<T, V>* right);
	private:
	};

	//叶子结点
	template <class T, class V>
	class LeafNode : public Node<T, V>
	{
	public:
		LeafNode(int degree);
		~LeafNode();

		T* find(V key);
		Node<T, V>* findLeafNode(V key);
		Node<T, V>* insert(T* value, V key);
		Node<T, V>* del(T* value, V key);
		LeafNode<T, V>* updateHead();
		
		T** values;
		LeafNode<T, V>* left;
		LeafNode<T, V>* right;
	private:
	};

	//B+树
	template <class T, class V>
	class BPlusTree
	{
	public:
		BPlusTree(int degree = 3);
		~BPlusTree();
		T* find(V key);
		LeafNode<T, V>* getHead();
		Node<T, V>* findLeafNode(V key);
		void insert(T* value, V key);
		void deleteNode(T* value, V key);
	private:
		//B+树的阶
		int degree;
		//最大结点数
		int maxNumber;
		//B+树的根
		Node<T, V>* root;
		//叶子结点的链表头
		LeafNode<T, V>* head;
	};

	/********************************* Node 函数实现 ******************************/

	template<class T, class V>
	Node<T, V>::Node(int degree) {
		this->degree = degree;
		this->parent = nullptr;
		this->numKids = 0;
		this->kids = new Node<T, V>*[degree + 1] { nullptr };
		this->keys = new V[degree + 1];
	}

	template <class T, class V>
	Node<T, V>::~Node() {
		if (keys != nullptr) {
			delete[] this->keys;
			keys = nullptr;
		}
		for (int i = 0; i < this->numKids; i++) {
			if (this->kids[i] != nullptr) {
				delete this->kids[i];
				this->kids[i] = nullptr;
			}
		}
		if (this->kids != nullptr) {
			delete[] this->kids;
			kids = nullptr;
		}
	}

	/********************************* B+树非叶子结点 函数实现 ******************************/

	template<class T, class V>
	BPlusTreeNode<T, V>::BPlusTreeNode(int degree) : Node<T, V>(degree) {}

	template<class T, class V>
	T* BPlusTreeNode<T, V>::find(V key) {
		int i = 0;
		while (i < this->numKids && this->keys[i] < key) {
			i++;
		}
		if (i == this->numKids) {
			return nullptr;
		}
		return this->kids[i]->find(key);
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::findLeafNode(V key) {
		int i = 0;
		while (i < this->numKids && this->keys[i] < key) {
			i++;
		}
		if (i == this->numKids) {
			i--;
		}
		return this->kids[i]->findLeafNode(key);
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::insert(T* value, V key) {
		int i = 0;
		while (i < this->numKids && this->keys[i] <= key) {
			i++;
		}
		//如果结点中所有值都比带插入 键 小，则在最右边结点孩子中执行插入
		if (i == this->numKids) {
			i--;
		}
		return this->kids[i]->insert(value, key);
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::del(T* value, V key) {
		int i = 0;
		while (i < this->numKids && this->keys[i] < key) {
			i++;
		}
		if (i == this->numKids) {
			return nullptr;
		}
		return this->kids[i]->del(value, key);
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::updateHead() {
		return this->kids[0]->updateHead();
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::insertKid(Node<T, V>* left, Node<T, V>* right, V maxNum) {
		V oldMaxKey = INT_MAX;
		if (this->numKids > 0) {
			oldMaxKey = this->keys[this->numKids - 1];
		}
		//如果该结点为新的根节点
		if (maxNum == INT_MAX || this->numKids <= 0)  {
			this->keys[0] = left->keys[left->numKids - 1];
			this->keys[1] = right->keys[right->numKids - 1];
			this->kids[0] = left;
			this->kids[1] = right;
			this->numKids = this->numKids + 2;
			return this;
		}
		//找到旧的最大值，并使用两个子结点的进行更替
		int i = 0;
		while (i < this->numKids && this->kids[i] != left) {
			i++;
		}
		this->keys[i] = left->keys[left->numKids - 1];
		this->kids[i] = left;
		i++;
		V rightKey = right->keys[right->numKids - 1];
		Node<T, V>* rightKid = right;
		while (i < this->numKids) {
			V tempKey = this->keys[i];
			Node<T, V>* tempKid = this->kids[i];
			this->keys[i] = rightKey;
			this->kids[i] = rightKid;
			rightKey = tempKey;
			rightKid = tempKid;
			i++;
		}
		this->keys[i] = rightKey;
		this->kids[i] = rightKid;
		this->numKids = this->numKids + 1;

		//结点不用拆分
		if (this->numKids <= this->degree) {
			//修改最大 键
			Node<T, V>* node = this;
			while (node->parent != nullptr) {
				V temp = node->keys[node->numKids - 1];
				if (temp > node->parent->keys[node->parent->numKids - 1]) {
					node->parent->keys[node->parent->numKids - 1] = temp;
					node = node->parent;
				} else {
					break;
				}
			}
			return nullptr;
		}

		//需要拆分
		int mid = this->numKids / 2;
		//配置新右节点
		BPlusTreeNode<T, V>* newNode = new BPlusTreeNode<T, V>(this->degree);
		newNode->numKids = this->numKids - mid;
		newNode->parent = this->parent;
		//如果父节点为空，即当前结点为根节点
		if (this->parent == nullptr) {
			BPlusTreeNode<T, V>* parent = new BPlusTreeNode<T, V>(this->degree);
			this->parent = parent;
			newNode->parent = parent;
			oldMaxKey = INT_MAX;
		}
		for (int i = 0; i < newNode->numKids; i++) {
			newNode->kids[i] = this->kids[mid + i];
			newNode->kids[i]->parent = newNode;		//***很重要
			newNode->keys[i] = this->keys[mid + i];
		}
		//重置旧结点
		for (int i = mid; i < this->numKids; i++) {
			this->kids[i] = nullptr;
		}
		this->numKids = mid;

		BPlusTreeNode<T, V>* parent = static_cast<BPlusTreeNode<T, V>*>(this->parent);
		return parent->insertKid(this, newNode, oldMaxKey);
	}

	template<class T, class V>
	Node<T, V>* BPlusTreeNode<T, V>::delKid(Node<T, V>* left, Node<T, V>* right) {
		V oldKey = this->keys[this->numKids - 1];
		//左节点保留，右节点删除
		int i = 0;
		while (i < this->numKids && this->kids[i] != left) {
			i++;
		}
		this->keys[i] = left->keys[left->numKids - 1];
		i++;	//当前的kids为right
		delete this->kids[i];	//删除right结点
		while (i < this->numKids - 1) {
			this->keys[i] = this->keys[i + 1];
			this->kids[i] = this->kids[i + 1];
			i++;
		}
		this->kids[i] = nullptr;
		this->numKids = this->numKids - 1;

		//不用 借键 或 合并结点
		if (this->numKids >= 2) {
			//旧的最大键 与 新的最大键 不同，则向上更新最大键
			if (oldKey != this->keys[this->numKids - 1]) {
				Node<T, V>* node = this;
				while (node->parent != nullptr) {
					V tempKey = node->keys[node->numKids - 1];
					Node<T, V>* parent = node->parent;
					int i = 0;
					while (i < parent->numKids && parent->kids[i] != node) {
						i++;
					}
					parent->keys[i] = tempKey;
					if (i != parent->numKids - 1) {
						break;
					}
					else {
						node = parent;
					}
				}
			}
			return nullptr;
		}

		//当前结点为根节点，没办法借键
		//（该结点里面的key只剩1个了，只有一个孩子，信息全在left中，则left作为新的根节点
		if (this->parent == nullptr) {
			this->kids[0]->parent = nullptr;
			this->kids[0] = nullptr;
			delete this;	//析构此对象后，不允许访问该对象的成员变量(可行)
			return left;
		}

		//需要借键
		Node<T, V>* parent = this->parent;
		i = 0;
		while (i < parent->numKids && parent->kids[i] != this) {
			i++;
		}
		//看看有没有左兄弟，并且可以借键的
		if (i > 0 && parent->kids[i - 1]->numKids > 2) {
			Node<T, V>* bro = parent->kids[i - 1];
			this->numKids = this->numKids + 1;
			int i = this->numKids - 1;
			while (i > 0) {
				this->keys[i] = this->keys[i - 1];
				this->kids[i] = this->kids[i - 1];
				i--;
			}
			this->keys[i] = bro->keys[bro->numKids - 1];
			this->kids[i] = bro->kids[bro->numKids - 1];
			this->kids[i]->parent = this->parent;
			bro->kids[bro->numKids - 1] = nullptr;
			bro->numKids = bro->numKids - 1;

			//更新 左兄弟节点 的父节点的索引（必不可能为父节点的最大键）
			i = 0;
			while (i < parent->numKids && parent->kids[i] != bro) {
				i++;
			}
			parent->keys[i] = bro->keys[bro->numKids - 1];

			return nullptr;
		}
		//看看有没有右兄弟，并且可以借键的
		if (i < parent->numKids - 1 && parent->kids[i + 1]->numKids > 2) {
			Node<T, V>* bro = parent->kids[i + 1];
			this->keys[this->numKids] = bro->keys[0];
			this->kids[this->numKids] = bro->kids[0];
			this->kids[this->numKids]->parent = this->parent;
			this->numKids = this->numKids + 1;

			int i = 0;
			while (i < bro->numKids - 1) {
				bro->keys[i] = bro->keys[i + 1];
				bro->kids[i] = bro->kids[i + 1];
				i++;
			}
			bro->kids[i] = nullptr;
			bro->numKids = bro->numKids - 1;

			//更新 当前节点 的父节点的索引（必不可能为父节点的最大键，因为有右兄弟）
			i = 0;
			while (i < parent->numKids && parent->kids[i] != this) {
				i++;
			}
			parent->keys[i] = this->keys[this->numKids - 1];

			return nullptr;
		}

		//左右兄弟都没有多余的键，则进行合并
		//先看看左兄弟
		Node<T, V>* leftNode{ nullptr };
		Node<T, V>* rightNode{ nullptr };
		if (i > 0) {
			leftNode = parent->kids[i - 1];
			rightNode = this;
		} else {	//没有左兄弟那必定有右兄弟
			leftNode = this;
			rightNode = parent->kids[i + 1];
		}
		//将右节点合并到左节点上
		i = 0;
		while (i < rightNode->numKids) {
			leftNode->keys[leftNode->numKids + i] = rightNode->keys[i];
			leftNode->kids[leftNode->numKids + i] = rightNode->kids[i];
			leftNode->kids[leftNode->numKids + i]->parent = leftNode;
			rightNode->kids[i] = nullptr;
			i++;
		}
		leftNode->numKids = leftNode->numKids + rightNode->numKids;

		BPlusTreeNode<T, V>* parent2 = static_cast<BPlusTreeNode<T, V>*>(parent);
		return parent2->delKid(leftNode, rightNode);

		//return nullptr;
	}
	
	/********************************* B+树叶子结点 函数实现 ******************************/

	template<class T, class V>
	LeafNode<T, V>::LeafNode(int degree) : Node<T, V>(degree)  {
		this->values = new T * [degree + 1]{ nullptr };
		this->left = nullptr;
		this->right = nullptr;
	}

	template<class T, class V>
	LeafNode<T, V>::~LeafNode() {
		for (int i = 0; i < this->numKids; i++) {
			this->values[i] = nullptr;
		}
		delete[] this->values;
	}

	template<class T, class V>
	T* LeafNode<T, V>::find(V key) {
		int i = 0;
		while (i < this->numKids && this->keys[i] < key) {
			i++;
		}
		if (i == this->numKids || this->keys[i] != key) {
			return nullptr;
		}
		return this->values[i];
	}

	template <class T, class V>
	Node<T, V>* LeafNode<T, V>::findLeafNode(V key) {
		return this;
	}

	template<class T, class V>
	Node<T, V>* LeafNode<T, V>::insert(T* value, V key) {
		//记录当前结点的最大的键
		V oldMaxKey = this->numKids > 0 ? this->keys[this->numKids - 1] : INT_MAX;

		//找到需要插入到该结点的位置
		int i = 0;
		while (i < this->numKids && this->keys[i] <= key) {
			i++;
		}
		//把数组元素后移（插入 键 值）
		while (i < this->numKids) {
			V tempKey = this->keys[i];
			T* tempValue = this->values[i];
			this->keys[i] = key;
			this->values[i] = value;
			key = tempKey;
			value = tempValue;
			i++;
		}
		this->keys[i] = key;
		this->values[i] = value;
		this->numKids = this->numKids + 1;
		
		//不需要进行结点拆分，直接返回
		if (this->numKids <= this->degree) {
			//如果插入的值为该结点的最大值，则更新该结点的所有父节点
			Node<T, V>* node = this;
			while (node->parent != nullptr) {
				V tempKey = node->keys[node->numKids - 1];
				if (tempKey > node->parent->keys[node->parent->numKids - 1]) {
					node->parent->keys[node->parent->numKids - 1] = tempKey;
					node = node->parent;
				} else {
					break;
				}
			}
			return nullptr;
		}
		
		//需要拆分结点
		int mid = this->numKids / 2;
		//创建又结点
		LeafNode<T, V>* newNode = new LeafNode<T, V>(this->degree);
		newNode->numKids = this->numKids - mid;
		newNode->parent = this->parent;
		//如果父节点为空，即根节点为叶子结点，需要创建父节点
		if (this->parent == nullptr) {
			BPlusTreeNode<T, V>* parent = new BPlusTreeNode<T, V>(this->degree);
			this->parent = parent;
			newNode->parent = parent;
			oldMaxKey = INT_MAX;
		}
		//将数据复制到新节点中
		for (int i = 0; i < newNode->numKids; i++) {
			newNode->keys[i] = this->keys[mid + i];
			newNode->values[i] = this->values[mid + i];
		}

		//修正旧结点数据
		for (int i = mid; i < this->numKids; i++) {
			this->values[i] = nullptr;
		}
		this->numKids = mid;

		newNode->right = this->right;
		newNode->left = this;
		if (newNode->right != nullptr) {
			newNode->right->left = newNode;
		}
		newNode->left->right = newNode;

		//将分裂后的两个结点插入到父节点中（由父节点来完成）
		BPlusTreeNode<T, V>* parent = static_cast<BPlusTreeNode<T, V>*>(this->parent);
		return parent->insertKid(this, newNode, oldMaxKey);
	}

	template<class T, class V>
	Node<T, V>* LeafNode<T, V>::del(T* value, V key) {
		//记录当前结点的最大的键
		V oldMaxKey = this->keys[this->numKids - 1];

		//找到需要删除的结点位置
		int i = 0;
		while (i < this->numKids && this->values[i] != value) {
			i++;
		}

		//无该结点
		if (i == this->numKids) {
			return nullptr;
		}

		//把数组元素前移（删除当前键值）
		while (i < this->numKids - 1) {
			this->keys[i] = this->keys[i + 1];
			this->values[i] = this->values[i + 1];
			i++;
		}
		this->values[i] = nullptr;
		this->numKids = this->numKids - 1;

		//一个结点最少拥有的键数
		int minNode = this->degree / 2;
		//当前结点数量大于 二分之阶 则无需合并，直接返回 
		if (this->numKids >= minNode) {
			//如果删除的键为该结点的最大值，需要更新父节点的索引
			if (key == oldMaxKey) {
				Node<T, V>* node = this;
				while (node->parent != nullptr) {
					V tempKey = node->keys[node->numKids - 1];
					Node<T, V>* parent = node->parent;
					int i = 0;
					while (i < parent->numKids && parent->kids[i] != node) {
						i++;
					}
					parent->keys[i] = tempKey;
					if (i != parent->numKids - 1) {
						break;
					} else {
						node = parent;
					}
				}
			}
			return nullptr;
		}

		//如果是根节点，则无任何 借键 / 合并 操作直接返回
		if (this->parent == nullptr) {
			return nullptr;
		}

		//需要 借键 或 合并结点（先看左兄弟，再看右兄弟）
		//如果有左兄弟
		if (this->left != nullptr && this->left->parent == this->parent) {
			//可以借键
			if (this->left->numKids > minNode) {
				LeafNode<T, V>* bro = this->left;
				this->numKids = this->numKids + 1;
				int i = this->numKids - 1;
				while (i > 0) {
					this->keys[i] = this->keys[i - 1];
					this->values[i] = this->values[i - 1];
					i--;
				}
				this->keys[i] = bro->keys[bro->numKids - 1];
				this->values[i] = bro->values[bro->numKids - 1];
				bro->values[bro->numKids - 1] = nullptr;
				bro->numKids = bro->numKids - 1;

				//更新 左兄弟节点 的父节点的索引（必不可能为父节点的最大键）
				Node<T, V>* parent = bro->parent;
				i = 0;
				while (i < parent->numKids && parent->kids[i] != bro) {
					i++;
				}
				parent->keys[i] = bro->keys[bro->numKids - 1];

				//当阶比较小时，当前结点删除节点后会变为空，再从左兄弟借一个键
				//此时右节点的最大值更新
				Node<T, V>* node = this;
				while (node->parent != nullptr) {
					V tempKey = node->keys[node->numKids - 1];
					Node<T, V>* parent = node->parent;
					int i = 0;
					while (i < parent->numKids && parent->kids[i] != node) {
						i++;
					}
					parent->keys[i] = tempKey;
					if (i != parent->numKids - 1) {
						break;
					} else {
						node = parent;
					}
				}

				return nullptr;
			}
		}
		//左兄弟没有多于的键，就看看右兄弟
		if (this->right != nullptr && this->right->parent == this->parent){	//没有左兄弟必定有右兄弟
			//可以借键
			if (this->right->numKids > minNode) {
				LeafNode<T, V>* bro = this->right;
				
				this->keys[this->numKids] = bro->keys[0];
				this->values[this->numKids] = bro->values[0];
				this->numKids = this->numKids + 1;

				int i = 0;
				while (i < bro->numKids - 1) {
					bro->keys[i] = bro->keys[i + 1];
					bro->values[i] = bro->values[i + 1];
					i++;
				}
				bro->values[i] = nullptr;
				bro->numKids = bro->numKids - 1;

				//更新 当前节点 的父节点的索引（必不可能为父节点的最大键，因为有右兄弟）
				Node<T, V>* parent = this->parent;
				i = 0;
				while (i < parent->numKids && parent->kids[i] != this) {
					i++;
				}
				parent->keys[i] = this->keys[this->numKids - 1];

				return nullptr;
			}
		}

		//如果左右兄弟都没有多余的键则进行结点合并
		//先看看左兄弟
		LeafNode<T, V>* leftNode{ nullptr };
		LeafNode<T, V>* rightNode{ nullptr };
		if (this->left != nullptr && this->left->parent == this->parent) {
			leftNode = this->left;
			rightNode = this;
		} else {	//没有左兄弟那必定有右兄弟
			leftNode = this;
			rightNode = this->right;
		}
		//将右节点合并到左节点上
		i = 0;
		while (i < rightNode->numKids) {
			leftNode->keys[leftNode->numKids + i] = rightNode->keys[i];
			leftNode->values[leftNode->numKids + i] = rightNode->values[i];
			rightNode->values[i] = nullptr;
			i++;
		}
		//更新leftNode基本信息以及指针
		leftNode->numKids = leftNode->numKids + rightNode->numKids;
		leftNode->right = rightNode->right;
		if (leftNode->right != nullptr) {
			leftNode->right->left = leftNode;
		}

		BPlusTreeNode<T, V>* parent = static_cast<BPlusTreeNode<T, V>*>(leftNode->parent);
		return parent->delKid(leftNode, rightNode);
	}

	template<class T, class V>
	LeafNode<T, V>* LeafNode<T, V>::updateHead() {
		if (this->numKids <= 0) {
			return nullptr;
		}
		return this;
	}

	/********************************* B+树 函数实现 ******************************/
	template <class T, class V>
	BPlusTree<T, V>::BPlusTree(int degree) {
		this->degree = degree;
		this->maxNumber = degree + 1;
		this->root = new LeafNode<T, V>(degree);
		this->head = nullptr;
	}

	template <class T, class V>
	BPlusTree<T, V>::~BPlusTree() {
		//后根遍历释放空间
		this->head = nullptr;
		delete root;
	}

	template <class T, class V>
	LeafNode<T, V>* BPlusTree<T, V>::getHead() {
		return this->head;
	}

	template <class T, class V>
	T* BPlusTree<T, V>::find(V key) {
		//T ret = root->find(key);
		//找不到则返回nullptr
		return root->find(key);
	}

	template <class T, class V>
	Node<T, V>* BPlusTree<T, V>::findLeafNode(V key) {
		return root->findLeafNode(key);
	}

	template <class T, class V>
	void BPlusTree<T, V>::insert(T* value, V key) {
		Node<T, V>* temp = root->insert(value, key);
		if (temp != nullptr) {
			this->root = temp;
		}
		this->head = static_cast<LeafNode<T, V>*>(this->root->updateHead());
	}

	template <class T, class V>
	void BPlusTree<T, V>::deleteNode(T* value, V key) {
		Node<T, V>* temp = root->del(value, key);
		if (temp != nullptr) {
			this->root = temp;
		}
		this->head = static_cast<LeafNode<T, V>*>(this->root->updateHead());
	}

}

