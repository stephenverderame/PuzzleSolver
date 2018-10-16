#pragma once
#include <initializer_list>
#include <vector>
#include <list>
#include <stack>
#include "Math.h"
#include <set>
/**
 * While parent is allocated memory, each instance has shared ownership of the resouce
 * Therefore the destructor is intentially left out and deallocation is handled manually
*/
namespace maze_helper {
	static int length = 0;
}
/**
 * Depends on global maze_helper::length
 * maze_helper::length must be initialized before a pos instance is created
 * Not the safest design decision but its for (much needed) efficiency
*/
struct pos {
	int x;
	int y;
	pos * parent;
	int id;
	pos() : parent(nullptr) {};
	pos(int x, int y) : x(x), y(y), parent(nullptr), id(y * maze_helper::length + x) {};
	inline bool operator==(const pos & other) const {
		return id == other.id;
	}
	inline bool operator!=(const pos & other) const {
		return id != other.id;
	}
	pos& operator=(const pos & other) {
		x = other.x;
		y = other.y;
		parent = other.parent;
		id = other.id;
		return *this;
	}
	inline pos& operator+(const pos & other) const {
		return pos{ x + other.x, y + other.y };
	}
	inline pos& operator+(const pos && other) const {
		return pos{ x + other.x, y + other.y };
	}
	inline pos& operator-(const pos & other) const {
		return pos{ x - other.x, y - other.y };
	}
	inline bool operator<(const pos & other) const {
		return id < other.id;
	}
	inline bool operator>(const pos & other) const {
		return id > other.id;
	}
};
/**
 * Map is not a resource owned by this class
*/
class Grid {
private:
	//* NO LONGER AN OWNED RESOURCE OF THIS CLASS
	int * map;
	int length;
	int height;
private:
	inline int diagnolHeuristic(const pos & current, const pos & goal) const noexcept {
		//for 8 directional mobility
		return Math::max(abs(current.x - goal.x), abs(current.y - goal.y));
	}
	inline int manhattenHeuristic(const pos & current, const pos & goal) const noexcept {
		//for 4 directional mobility
		return abs(current.x - goal.x) + abs(current.y - goal.y);
	}
	inline int distanceHeuristic(const pos & current, const pos & goal) const {
		return floorl(sqrt(pow(current.x - goal.x, 2) + pow(current.y - goal.y, 2)));
	}
public:
	/**
	 * @param length, height Length and height of image
	 * @param map result of data() method on std::vector. Map is not managed by this class
	*/
	Grid(int length, int height, int * map) : length(length), height(height), map(map) {
		maze_helper::length = length;
	}
	inline int get(int x, int y) const {
		return map[y * length + x];
	}
	inline void set(int x, int y, int weight) {
		map[y * length + x] = weight;
	}
	std::stack<std::pair<int, int>> search(const pos & start, const pos & goal) const {
		//implementing A*
		std::set<pos> open, closed;
		//Handles memory deallocation
		std::vector<pos*> allocatedMemory;

		//rough estimates of memory needed
		allocatedMemory.reserve(length * height * 0.15);
		int memIndex = -1;
		open.insert(start);
		pos current = start;
		while (++memIndex, !open.empty()) {
			std::pair<pos, int> minF = std::make_pair(pos{ 0, 0 }, INT_MAX);
			for (auto it = open.begin(); it != open.end(); ++it) {
				pos p = *it;
				int f = map[p.id] + diagnolHeuristic(p, goal);
				if (f < minF.second)
					minF = std::make_pair(p, f);
			}

			open.erase(minF.first);
			if (minF.first == goal) {
				closed.insert(minF.first);
				break;
			}
			allocatedMemory.push_back(new pos(minF.first));
			for (int i = -1; i <= 1; ++i) {
				for (int j = -1; j <= 1; ++j) {
					if ((i == 0 && j == 0) || minF.first.x + i < 0 || minF.first.x + i >= length || minF.first.y + j < 0 || minF.first.y + j >= height) continue;
					pos s = minF.first + pos{ i, j };					
					s.parent = allocatedMemory[memIndex];
					if (closed.find(s) != closed.end()) continue;
					open.insert(s);
				}
			}
			closed.insert(minF.first);

		}
		std::stack<std::pair<int, int>> path;
		pos c;
		c = *closed.find(goal);
		while (c != start) {
			path.push(std::make_pair(c.x, c.y));
 			c = *c.parent;
		}
		for (int i = 0; i < allocatedMemory.size(); ++i)
			delete allocatedMemory[i];
		path.push(std::make_pair(start.x, start.y));
		return path;
	}


};