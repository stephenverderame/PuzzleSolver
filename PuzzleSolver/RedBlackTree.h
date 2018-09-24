#pragma once
#include <queue>
#include <vector>
namespace RB {
	template<class T, typename P, typename = void>
	struct is_comparable : std::false_type {};
	template<typename... Ts>
	struct is_comparable_helper {};
	template<class T, typename P>
	struct is_comparable<T, P, std::void_t<
		is_comparable_helper<
			decltype(std::declval<P>() < std::declval<P>()),
			decltype(std::declval<P>() > std::declval<P>()),
			decltype(std::declval<T>().greaterThan(std::declval<P>(), std::declval<P>())),
			decltype(std::declval<T>().lessThan(std::declval<P>(), std::declval<P>()))
		>
	>> : std::true_type {};


	template<typename T>
	class RBCompare {
	private:
		virtual bool lessThanP(const T & a, const T & b) const noexcept {
			return a < b;
		}
		virtual bool greaterThanP(const T & a, const T & b) const noexcept {
			return a > b;
		}
	public:
		bool lessThan(const T & a, const T & b) const noexcept {
			return lessThanP(a, b);
		}
		bool greaterThan(const T & a, const T & b) const noexcept {
			return greaterThanP(a, b);
		}
	};
	template <typename T, class CType>
	class RedBlackTree {
	private:
		enum class color {
			red,
			black
		};
		template<typename P>
		struct node {
			node<P> * parent;
			node<P> * left, *right;
			color nodeColor;
			P data;
			int count;
		};
		node<T> * root;
		node<T> * nil;
		std::unique_ptr<CType> compareObj;
		int totalNodes;
	private:
		void leftRotate(node<T> * x) {
			node<T> * y = x->right;
			x->right = y->left;
			if (y->left != nil)
				y->left->parent = x;
			y->parent = x->parent;
			if (x->parent == nil)
				root = y;
			else if (x == x->parent->left)
				x->parent->left = y;
			else
				x->parent->right = y;
			y->left = x;
			x->parent = y;

		}
		void rightRotate(node<T> * x) {
			node<T> * y = x->left;
			x->left = y->right;
			if (y->right != nil)
				y->right->parent = x;
			y->parent = x->parent;
			if (x->parent == nil)
				root = y;
			else if (x == x->parent->left)
				x->parent->left = y;
			else
				x->parent->right = y;
			y->right = x;
			x->parent = y;
		}
		node<T> * newNode(T data) {
			node<T> * n = new node<T>{ nullptr, nil, nil, color::red, data, 0 };
			return n;
		}
		void fixTree(node<T> * n) {
			while (n->parent->nodeColor == color::red) {
				if (n->parent->parent != nullptr && n->parent == n->parent->parent->left) {
					node<T> * uncle = n->parent->parent->right;
					if (uncle->nodeColor == color::red) {
						n->parent->nodeColor = color::black;
						uncle->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						n = n->parent->parent;
					}
					else if (n == n->parent->right) {
						n = n->parent;
						leftRotate(n);
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						rightRotate(n->parent->parent);
					}
				}
				else if (n->parent->parent != nullptr) {
					node<T> * uncle = n->parent->parent->left;
					if (uncle->nodeColor == color::red) {
						n->parent->nodeColor = color::black;
						uncle->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						n = n->parent->parent;
					}
					else if (n == n->parent->left) {
						n = n->parent;
						rightRotate(n);
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						leftRotate(n->parent->parent);
					}
				}
			}
			root->nodeColor = color::black;
		}
		void insert(node<T> * z) {
			node<T> * prev = nil;
			node<T> * ptr = root;
			while (ptr != nil) {
				prev = ptr;
				if (compareObj->lessThan(z->data, ptr->data))
					ptr = ptr->left;
				else if (compareObj->greaterThan(z->data > ptr->data))
					ptr = ptr->right;
				else {
					++ptr->count;
					return;
				}
			}
			z->parent = prev;
			if (prev == nil)
				root = z;
			else if (z->data < prev->data)
				prev->left = z;
			else
				prev->right = z;
			fixTree(z);
		}
		int height(node<T> * root) {
			if (root == nullptr) return 0;
			auto leftHeight = height(root->left);
			auto rightHeight = height(root->right);
			return leftHeight > rightHeight ? leftHeight + 1 : rightHeight + 1;
		}
		void inorder(node<T> * n, std::vector<T> & out) {
			if (n != nil) {
				inorder(n->left, out);
				std::vector<T> fillVec(n->count, n->data);
				out.insert(out.end(), fillVec);
				inorder(n->right, out);
			}
		}
		void freeNode(node<T> * n) {
			if (n != nil) {
				freeNode(n->left);
				freeNode(n->right);
				delete n;
			}
		}
	public:
		RedBlackTree() : totalNodes(0) {
			static_assert(is_comparable<CType, T>::value, "RBTree's compare type must subclass RBCompare and its parameter must be comparable");
			compareObj = std::unique_ptr<CType>(new CType());
			nil = new node<T>{ nullptr, nullptr, nullptr, color::black, T(), 0 };
			root = nil;
		};
		~RedBlackTree() {
			freeNode(root);
			delete nil;
		}
		RedBlackTree& operator=(const RedBlackTree &) = delete; //not needed right now
		RedBlackTree(const RedBlackTree &) = delete;
		void insert(T t) {
			insert(newNode(t));
			++totalNodes;
		}
		int treeHeight() {
			return height(root);
		}
		int getCount() {
			return totalNodes;
		}
		std::vector<T>& inorderList() {
			std::vector<T> vec;
			vec.reserve(totalNodes);
			inorder(root, vec);
			return vec;
		}
		void breadthTraversal(std::ostream & out) const {
			if (root == nil) return;
			std::queue<node<T>*> q;
			node<T> * ptr;
			q.push(root);
			while (!q.empty()) {
				ptr = q.front();
				if (ptr->left != nil)
					q.push(ptr->left);
				if (ptr->right != nil)
					q.push(ptr->right);
				q.pop();
				out << ptr->data << " ";
			}
		}
	};
}