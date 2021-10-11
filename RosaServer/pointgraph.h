#pragma once
#include <cstdint>
#include <tuple>
#include <unordered_map>
#include <vector>
#include "sol/sol.hpp"

struct Link {
	unsigned int toId;
	int cost;
	Link(unsigned int toId, int cost) : toId(toId), cost(cost){};
};

struct NodePoint {
	int x, y, z;

	bool operator==(const NodePoint& other) const {
		return x == other.x && y == other.y && z == other.z;
	}
};

namespace std {
template <>
struct hash<NodePoint> {
	std::size_t operator()(const NodePoint& key) const {
		return hash<int>()(key.x) + 17 * hash<int>()(key.y) +
		       37 * hash<int>()(key.z);
	}
};
}  // namespace std

struct Node {
	NodePoint point;
	std::vector<Link> links;
	Node(NodePoint& point) : point(point){};
};

class PointGraph {
	std::vector<Node> nodes;
	std::unordered_map<NodePoint, unsigned int> nodeIdByPoint;
	double* squareRootCache;
	unsigned int numCachedSquareRoots;

 public:
	PointGraph(unsigned int squareRootCacheSize);
	~PointGraph();
	int getSize() const;
	void addNode(int x, int y, int z);
	std::tuple<int, int, int> getNodePoint(unsigned int index) const;
	void addLink(unsigned int fromId, unsigned int toId, int cost);
	sol::object getNodeByPoint(int x, int y, int z, sol::this_state s) const;
	sol::object findShortestPath(unsigned int startNodeId,
	                             unsigned int goalNodeId,
	                             sol::this_state s) const;
};