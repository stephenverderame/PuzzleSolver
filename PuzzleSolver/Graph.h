#pragma once
#include "img.h"
#include "Math.h"
using uchar = unsigned char;
class Graph {
private:
	std::vector<uchar> adjacencyMatrix;
	int nodes;
	int totalNodes;
private:
	bool isPath(int fromNode, int toNode) {
		int byteIndex = fromNode * totalNodes + toNode;
		uchar byte = adjacencyMatrix[byteIndex / 8];
		uchar bitIndex = byteIndex % 8;
		return (bool)(byte & bitIndex);
	}
public:
	Graph(int nodes) : nodes(ceil(nodes / 8)), totalNodes(nodes) {
		adjacencyMatrix.resize(ceil(nodes / 8) * ceil(nodes / 8), 0);
	}

};