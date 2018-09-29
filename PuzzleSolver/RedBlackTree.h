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
		int sumOfCounts;
	private:
		void leftRotate(node<T> * x) { // node x is the upper node; "pivot point"
			node<T> * y = x->right;
			x->right = y->left;
			if (y->left != nil && y->left != nullptr)
				y->left->parent = x;
			y->parent = x->parent;
			if (x->parent == nil || x->parent == nullptr)
				root = y;
			else if (x == x->parent->left)
				x->parent->left = y;
			else
				x->parent->right = y;
			y->left = x;
			x->parent = y;

		}
		void rightRotate(node<T> * x) { //node x is the upper node; "pivot point", lower node "rotates" around pivot to the right so that x becomes left child of y
			node<T> * y = x->left;
			x->left = y->right;
			if (y->right != nil && y->right != nullptr)
				y->right->parent = x;
			y->parent = x->parent;
			if (x->parent == nil || x->parent == nullptr)
				root = y;
			else if (x == x->parent->left)
				x->parent->left = y;
			else
				x->parent->right = y;
			y->right = x;
			x->parent = y;
		}
		node<T> * newNode(T data) {
			node<T> * n = new node<T>{ nullptr, nil, nil, color::red, data, 1 };
			return n;
		}
		void fixTree(node<T> * n) {
			/*	(remember all nodes have empty black children; a left rotation switches right child with pivot because the child is moving around the pivot leftwards; 
				pivot denoted with 'on')				
				General steps to fixing tree: 
				1. If root node - color it black
				2. If parent is black - ignore
				3. If parent and uncle is red - repaint parent and uncle to black, repaint grandparent to red and run grandparent through procedure
				4. If parent is red and uncle is black - rotate on parent to switch new node and parent
					Select child node (original parent node)
					5. Falls through from 4
					Rotate on grandparent to switch parent and grandparent
					Set parent (current grandparent) color to black and grandparent (current parent) to red 
							OR
					Before rotation set actual current parent black and actual current grandparent red
					*Either way, before or after rotation, you are simply switching color of parent and grandparent
			*/
			while (n->parent->nodeColor == color::red && n->parent != root) {
				if (n->parent == n->parent->parent->left) { //parent is left child
					node<T> * uncle = n->parent->parent->right;
					if (uncle->nodeColor == color::red) { //case 3 - parent and uncle are red, recolor parent and uncle and then run gparent through fixtree
						n->parent->nodeColor = color::black;
						uncle->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						n = n->parent->parent;
					}
					else if (n == n->parent->right) { //case 4 - falls through to 5 - parent is red and uncle is black, rotate to switch n and its parent (4)														
//						n = n->parent;				  //case 5 - rotate on grandparent to make the former parent parent of both grandparent and new node. and switch color of parent and grandparent
						leftRotate(n->parent);
						n = n->left;
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						rightRotate(n->parent->parent);
					}
					else if (n == n->parent->left) {
						rightRotate(n->parent);
						n = n->right;
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						rightRotate(n->parent->parent);
					}
				}
				else { //parent is right child
					node<T> * uncle = n->parent->parent->left;
					if (uncle->nodeColor == color::red) {
						n->parent->nodeColor = color::black;
						uncle->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						n = n->parent->parent;
					}
					else if (n == n->parent->left) {
//						n = n->parent;
						rightRotate(n->parent);
						n = n->right;
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						leftRotate(n->parent->parent);
					}
					else if (n == n->parent->right) {
						leftRotate(n->parent);
						n = n->left;
						n->parent->nodeColor = color::black;
						n->parent->parent->nodeColor = color::red;
						leftRotate(n->parent->parent);
					}
				}
			}
			root->nodeColor = color::black; //case 1 + 2, if root it must be black(1), or if parent is black do nothing (2)
		}
		void insert(node<T> * z) {
			node<T> * prev = nil;
			node<T> * ptr = root;
			while (ptr != nil) {
				prev = ptr;
				if (compareObj->lessThan(z->data, ptr->data))
					ptr = ptr->left;
				else if (compareObj->greaterThan(z->data, ptr->data))
					ptr = ptr->right;
				else {
					++(ptr->count);
					return;
				}
			}
			z->parent = prev;
			if (prev == nil)
				root = z;
			else if (compareObj->lessThan(z->data, prev->data))
				prev->left = z;
			else
				prev->right = z;
			fixTree(z);
			++totalNodes;
		}
		int height(node<T> * root) const {
			if (root == nil) return 0;
			auto leftHeight = height(root->left);
			auto rightHeight = height(root->right);
			return leftHeight > rightHeight ? leftHeight + 1 : rightHeight + 1;
		}
		void inorder(node<T> * n, std::vector<T> & out) const {
			if (n != nil) {
				inorder(n->left, out);
				std::vector<T> fillVec(n->count, n->data);
				out.insert(out.end(), fillVec.begin(), fillVec.end());
				inorder(n->right, out);
			}
		}
		void inorder_nor(node<T> * n, std::vector<T> & out) const {
			if (n != nil) {
				inorder_nor(n->left, out);
				out.push_back(n->data);
				inorder_nor(n->right, out);
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
		RedBlackTree() : totalNodes(0), sumOfCounts(0) {
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
			++sumOfCounts;
		}
		int treeHeight() const noexcept {
			return height(root);
		}
		int getTreeSize() const noexcept {
			return totalNodes;
		}
		int getTreeCount() const noexcept {
			return sumOfCounts;
		}
		std::vector<T> inorderList() const {
			std::vector<T> vec;
			vec.reserve(sumOfCounts);
			inorder(root, vec);
			return vec;
		}
		std::vector<T> inorderListNR() const {
			std::vector<T> vec;
			vec.reserve(totalNodes);
			inorder_nor(root, vec);
			return vec;
		}
		void breadthTraversal(std::ostream & out) const {
			if (root == nil || root == nullptr) return;
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
	template<typename T, typename = void>
	struct is_outputable : std::false_type {};
	template<typename T>
	struct is_outputable<T, std::void_t<decltype(std::declval<std::ostream>() << std::declval<T>())>> : std::true_type {};

	template<typename T, class CType>
	std::ostream& operator<<(std::ostream & out, const RB::RedBlackTree<T, CType> & tree) {
		static_assert(is_outputable<T>::value, "RBTree << operator attempted with objects that do not support << operator");
		std::vector<T> list = tree.inorderList();
		for (T & t : list)
			out << t << " ";
		return out;
	}
}