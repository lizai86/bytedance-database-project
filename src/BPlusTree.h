#pragma once
#include <limits.h>

namespace Bplustree {
	//��㣨��Ҷ�ӽ���Ҷ�ӽ�㣩
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
		
		//���ڵ�
		Node<T, V>* parent;
		//���ӽڵ�
		Node<T, V>** kids;
		//��
		V* keys;
		//���ӽڵ�ĸ���
		int numKids;
		//���Ķ�
		int degree;
	private:
	};

	//��Ҷ�ӽ��
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

	//Ҷ�ӽ��
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

	//B+��
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
		//B+���Ľ�
		int degree;
		//�������
		int maxNumber;
		//B+���ĸ�
		Node<T, V>* root;
		//Ҷ�ӽ�������ͷ
		LeafNode<T, V>* head;
	};

	/********************************* Node ����ʵ�� ******************************/

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

	/********************************* B+����Ҷ�ӽ�� ����ʵ�� ******************************/

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
		//������������ֵ���ȴ����� �� С���������ұ߽�㺢����ִ�в���
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
		//����ý��Ϊ�µĸ��ڵ�
		if (maxNum == INT_MAX || this->numKids <= 0)  {
			this->keys[0] = left->keys[left->numKids - 1];
			this->keys[1] = right->keys[right->numKids - 1];
			this->kids[0] = left;
			this->kids[1] = right;
			this->numKids = this->numKids + 2;
			return this;
		}
		//�ҵ��ɵ����ֵ����ʹ�������ӽ��Ľ��и���
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

		//��㲻�ò��
		if (this->numKids <= this->degree) {
			//�޸���� ��
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

		//��Ҫ���
		int mid = this->numKids / 2;
		//�������ҽڵ�
		BPlusTreeNode<T, V>* newNode = new BPlusTreeNode<T, V>(this->degree);
		newNode->numKids = this->numKids - mid;
		newNode->parent = this->parent;
		//������ڵ�Ϊ�գ�����ǰ���Ϊ���ڵ�
		if (this->parent == nullptr) {
			BPlusTreeNode<T, V>* parent = new BPlusTreeNode<T, V>(this->degree);
			this->parent = parent;
			newNode->parent = parent;
			oldMaxKey = INT_MAX;
		}
		for (int i = 0; i < newNode->numKids; i++) {
			newNode->kids[i] = this->kids[mid + i];
			newNode->kids[i]->parent = newNode;		//***����Ҫ
			newNode->keys[i] = this->keys[mid + i];
		}
		//���þɽ��
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
		//��ڵ㱣�����ҽڵ�ɾ��
		int i = 0;
		while (i < this->numKids && this->kids[i] != left) {
			i++;
		}
		this->keys[i] = left->keys[left->numKids - 1];
		i++;	//��ǰ��kidsΪright
		delete this->kids[i];	//ɾ��right���
		while (i < this->numKids - 1) {
			this->keys[i] = this->keys[i + 1];
			this->kids[i] = this->kids[i + 1];
			i++;
		}
		this->kids[i] = nullptr;
		this->numKids = this->numKids - 1;

		//���� ��� �� �ϲ����
		if (this->numKids >= 2) {
			//�ɵ����� �� �µ����� ��ͬ�������ϸ�������
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

		//��ǰ���Ϊ���ڵ㣬û�취���
		//���ý�������keyֻʣ1���ˣ�ֻ��һ�����ӣ���Ϣȫ��left�У���left��Ϊ�µĸ��ڵ�
		if (this->parent == nullptr) {
			this->kids[0]->parent = nullptr;
			this->kids[0] = nullptr;
			delete this;	//�����˶���󣬲�������ʸö���ĳ�Ա����(����)
			return left;
		}

		//��Ҫ���
		Node<T, V>* parent = this->parent;
		i = 0;
		while (i < parent->numKids && parent->kids[i] != this) {
			i++;
		}
		//������û�����ֵܣ����ҿ��Խ����
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

			//���� ���ֵܽڵ� �ĸ��ڵ���������ز�����Ϊ���ڵ��������
			i = 0;
			while (i < parent->numKids && parent->kids[i] != bro) {
				i++;
			}
			parent->keys[i] = bro->keys[bro->numKids - 1];

			return nullptr;
		}
		//������û�����ֵܣ����ҿ��Խ����
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

			//���� ��ǰ�ڵ� �ĸ��ڵ���������ز�����Ϊ���ڵ����������Ϊ�����ֵܣ�
			i = 0;
			while (i < parent->numKids && parent->kids[i] != this) {
				i++;
			}
			parent->keys[i] = this->keys[this->numKids - 1];

			return nullptr;
		}

		//�����ֵܶ�û�ж���ļ�������кϲ�
		//�ȿ������ֵ�
		Node<T, V>* leftNode{ nullptr };
		Node<T, V>* rightNode{ nullptr };
		if (i > 0) {
			leftNode = parent->kids[i - 1];
			rightNode = this;
		} else {	//û�����ֵ��Ǳض������ֵ�
			leftNode = this;
			rightNode = parent->kids[i + 1];
		}
		//���ҽڵ�ϲ�����ڵ���
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
	
	/********************************* B+��Ҷ�ӽ�� ����ʵ�� ******************************/

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
		//��¼��ǰ�������ļ�
		V oldMaxKey = this->numKids > 0 ? this->keys[this->numKids - 1] : INT_MAX;

		//�ҵ���Ҫ���뵽�ý���λ��
		int i = 0;
		while (i < this->numKids && this->keys[i] <= key) {
			i++;
		}
		//������Ԫ�غ��ƣ����� �� ֵ��
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
		
		//����Ҫ���н���֣�ֱ�ӷ���
		if (this->numKids <= this->degree) {
			//��������ֵΪ�ý������ֵ������¸ý������и��ڵ�
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
		
		//��Ҫ��ֽ��
		int mid = this->numKids / 2;
		//�����ֽ��
		LeafNode<T, V>* newNode = new LeafNode<T, V>(this->degree);
		newNode->numKids = this->numKids - mid;
		newNode->parent = this->parent;
		//������ڵ�Ϊ�գ������ڵ�ΪҶ�ӽ�㣬��Ҫ�������ڵ�
		if (this->parent == nullptr) {
			BPlusTreeNode<T, V>* parent = new BPlusTreeNode<T, V>(this->degree);
			this->parent = parent;
			newNode->parent = parent;
			oldMaxKey = INT_MAX;
		}
		//�����ݸ��Ƶ��½ڵ���
		for (int i = 0; i < newNode->numKids; i++) {
			newNode->keys[i] = this->keys[mid + i];
			newNode->values[i] = this->values[mid + i];
		}

		//�����ɽ������
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

		//�����Ѻ�����������뵽���ڵ��У��ɸ��ڵ�����ɣ�
		BPlusTreeNode<T, V>* parent = static_cast<BPlusTreeNode<T, V>*>(this->parent);
		return parent->insertKid(this, newNode, oldMaxKey);
	}

	template<class T, class V>
	Node<T, V>* LeafNode<T, V>::del(T* value, V key) {
		//��¼��ǰ�������ļ�
		V oldMaxKey = this->keys[this->numKids - 1];

		//�ҵ���Ҫɾ���Ľ��λ��
		int i = 0;
		while (i < this->numKids && this->values[i] != value) {
			i++;
		}

		//�޸ý��
		if (i == this->numKids) {
			return nullptr;
		}

		//������Ԫ��ǰ�ƣ�ɾ����ǰ��ֵ��
		while (i < this->numKids - 1) {
			this->keys[i] = this->keys[i + 1];
			this->values[i] = this->values[i + 1];
			i++;
		}
		this->values[i] = nullptr;
		this->numKids = this->numKids - 1;

		//һ���������ӵ�еļ���
		int minNode = this->degree / 2;
		//��ǰ����������� ����֮�� ������ϲ���ֱ�ӷ��� 
		if (this->numKids >= minNode) {
			//���ɾ���ļ�Ϊ�ý������ֵ����Ҫ���¸��ڵ������
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

		//����Ǹ��ڵ㣬�����κ� ��� / �ϲ� ����ֱ�ӷ���
		if (this->parent == nullptr) {
			return nullptr;
		}

		//��Ҫ ��� �� �ϲ���㣨�ȿ����ֵܣ��ٿ����ֵܣ�
		//��������ֵ�
		if (this->left != nullptr && this->left->parent == this->parent) {
			//���Խ��
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

				//���� ���ֵܽڵ� �ĸ��ڵ���������ز�����Ϊ���ڵ��������
				Node<T, V>* parent = bro->parent;
				i = 0;
				while (i < parent->numKids && parent->kids[i] != bro) {
					i++;
				}
				parent->keys[i] = bro->keys[bro->numKids - 1];

				//���ױȽ�Сʱ����ǰ���ɾ���ڵ����Ϊ�գ��ٴ����ֵܽ�һ����
				//��ʱ�ҽڵ�����ֵ����
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
		//���ֵ�û�ж��ڵļ����Ϳ������ֵ�
		if (this->right != nullptr && this->right->parent == this->parent){	//û�����ֵܱض������ֵ�
			//���Խ��
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

				//���� ��ǰ�ڵ� �ĸ��ڵ���������ز�����Ϊ���ڵ����������Ϊ�����ֵܣ�
				Node<T, V>* parent = this->parent;
				i = 0;
				while (i < parent->numKids && parent->kids[i] != this) {
					i++;
				}
				parent->keys[i] = this->keys[this->numKids - 1];

				return nullptr;
			}
		}

		//��������ֵܶ�û�ж���ļ�����н��ϲ�
		//�ȿ������ֵ�
		LeafNode<T, V>* leftNode{ nullptr };
		LeafNode<T, V>* rightNode{ nullptr };
		if (this->left != nullptr && this->left->parent == this->parent) {
			leftNode = this->left;
			rightNode = this;
		} else {	//û�����ֵ��Ǳض������ֵ�
			leftNode = this;
			rightNode = this->right;
		}
		//���ҽڵ�ϲ�����ڵ���
		i = 0;
		while (i < rightNode->numKids) {
			leftNode->keys[leftNode->numKids + i] = rightNode->keys[i];
			leftNode->values[leftNode->numKids + i] = rightNode->values[i];
			rightNode->values[i] = nullptr;
			i++;
		}
		//����leftNode������Ϣ�Լ�ָ��
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

	/********************************* B+�� ����ʵ�� ******************************/
	template <class T, class V>
	BPlusTree<T, V>::BPlusTree(int degree) {
		this->degree = degree;
		this->maxNumber = degree + 1;
		this->root = new LeafNode<T, V>(degree);
		this->head = nullptr;
	}

	template <class T, class V>
	BPlusTree<T, V>::~BPlusTree() {
		//��������ͷſռ�
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
		//�Ҳ����򷵻�nullptr
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

