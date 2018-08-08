#pragma once
#include <initializer_list>
#include <vector>
#include <list>
#include <stack>
#define MAX(X, Y) (X) > (Y) ? (X) : (Y)
struct pos {
	int x;
	int y;
	pos * parent;
	pos() {};
	pos(int x, int y) : x(x), y(y) {};
	bool operator==(pos other) {
		return x == other.x && y == other.y;
	}
	bool operator!=(pos other) {
		return !(*this == other);
	}
	pos operator=(pos other) {
		x = other.x;
		y = other.y;
		parent = other.parent;
		return *this;
	}
	pos operator+(pos other) {
		return pos{ x + other.x, y + other.y };
	}
	pos operator-(pos other) {
		return pos{ x - other.x, y - other.y };
	}
};
class Grid {
private:
	int * map;
	int length;
	int height;
	pos crop1;
	pos crop2;
	bool cropped;
private:
	int diagnolHeuristic(pos current, pos goal) {
		//for 8 directional mobility
		return MAX(abs(current.x - goal.x), abs(current.y - goal.y));
	}
	int manhattenHeuristic(pos current, pos goal) {
		//for 4 directional mobility
		return abs(current.x - goal.x) + abs(current.y - goal.y);
	}
	int distanceHeuristic(pos current, pos goal) {
		return floorl(sqrt(pow(current.x - goal.x, 2) + pow(current.y - goal.y, 2)));
	}
public:
	Grid(int length, int height, std::initializer_list<int> l) : length(length), height(height), cropped(false) {
		map = new int[length * height];
		int i = 0;
		for (auto it = l.begin(); it != l.end(); it++) {
			map[i] = *it;
			i++;
		}

	};
	Grid(int length, int height) : length(length), height(height), cropped(false) {
		map = new int[length * height];
	};
	Grid(int length, int height, int * map) : length(length), height(height), map(map), cropped(false) {}
	~Grid() {
		delete map;
	}
	int get(int x, int y) {
		return map[y * length + x];
	}
	void setBound(pos p1, pos p2) {
		crop1 = p1;
		crop2 = p2;
		cropped = true;
	}
	void set(int x, int y, int weight) {
		map[y * length + x] = weight;
	}
	std::stack<pos> search(pos start, pos goal) {
		//implementing A*
		std::list<pos> open;
		std::vector<pos> closed;
		open.push_back(start);
		pos current = start;
		while (!open.empty()) {
			std::pair<pos, int> minF = std::make_pair(pos{ 0, 0 }, INT_MAX);
			for (auto it = open.begin(); it != open.end(); it++) {
				//				printf("loop\n");
				pos p = *it;
				int f = get(p.x, p.y) + diagnolHeuristic(p, goal);
				//				printf("(%d, %d) F: %d\n", p.x, p.y, f);
				if (f < minF.second)
					minF = std::make_pair(p, f);
			}

			//			printf("Point with least f: (%d, %d) of %d \n", minF.first.x, minF.first.y, minF.second);
			open.remove(minF.first);
			if (minF.first == goal) {
				closed.push_back(minF.first);
				break;
			}
			std::vector<pos> successors;
			for (int i = -1; i <= 1; i++) {
				for (int j = -1; j <= 1; j++) {
					if (i == 0 && j == 0) continue;
					pos s = minF.first + pos{ i, j };
					s.parent = new pos(minF.first);
					successors.push_back(s);
				}
			}
			for (auto it = successors.begin(); it != successors.end(); it++) {
				pos p = *it;
				if (p.x < 0 || p.x >= length || p.y < 0 || p.y >= height) continue;
				if (cropped && (p.x < min(crop1.x, crop2.x) || p.x > max(crop1.x, crop2.x) || p.y < min(crop1.y, crop2.y) || p.y > max(crop1.y, crop2.y))) continue;
				bool cont = false;
				for (auto itt = open.begin(); itt != open.end(); itt++) {
					if (p == *itt/* && f > get((*itt).x, (*itt).y) + diagnolHeuristic(*itt, goal)*/) {
						cont = true;
						break;
					}
				}
				if (cont) continue;
				cont = false;
				for (auto itt = closed.begin(); itt != closed.end(); itt++) {
					if (p == *itt/* && f > get((*itt).x, (*itt).y) + diagnolHeuristic(*itt, goal)*/) {
						cont = true;
						break;
					}
				}
				if (cont) continue;
				open.push_back(p);
			}
			closed.push_back(minF.first);

		}
		std::stack<pos> path;
		pos c;
		for (auto it = closed.begin(); it != closed.end(); it++) {
			if (*it == goal) {
				c = *it;
				break;
			}
		}
		while (c != start) {
			path.push(c);
			c = *c.parent;
		}
		path.push(start);
		return path;
	}


};