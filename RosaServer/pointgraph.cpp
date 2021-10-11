#include "pointgraph.h"
#include <queue>
#include <stdexcept>
#include <unordered_set>

PointGraph::PointGraph(unsigned int squareRootCacheSize) {
	squareRootCache = new double[squareRootCacheSize];
	numCachedSquareRoots = squareRootCacheSize;

	for (unsigned int square = 0; square < squareRootCacheSize; square++) {
		squareRootCache[square] = std::sqrt(square);
	}
}

PointGraph::~PointGraph() { delete[] squareRootCache; }

int PointGraph::getSize() const { return nodes.size(); }

void PointGraph::addNode(const int x, const int y, const int z) {
	NodePoint point{x, y, z};
	nodes.emplace_back(point);
	nodeIdByPoint.insert({point, nodes.size() - 1});
}

std::tuple<int, int, int> PointGraph::getNodePoint(unsigned int index) const {
	const Node& node = nodes.at(index);
	return std::make_tuple(node.point.x, node.point.y, node.point.z);
}

void PointGraph::addLink(unsigned int fromId, unsigned int toId, int cost) {
	Node& node = nodes.at(fromId);
	if (toId >= nodes.size()) {
		throw std::invalid_argument("Link isn't to a valid node");
	}
	node.links.emplace_back(toId, cost);
}

sol::object PointGraph::getNodeByPoint(int x, int y, int z,
                                       sol::this_state s) const {
	sol::state_view lua(s);

	const auto search = nodeIdByPoint.find({x, y, z});
	if (search != nodeIdByPoint.end()) {
		return sol::make_object(lua, search->second);
	} else {
		return sol::make_object(lua, sol::nil);
	}
}

static inline double getHeuristicScore(const Node& node, const Node& goalNode,
                                       const unsigned int numCachedSquareRoots,
                                       const double* squareRootCache) {
	const int deltaX = goalNode.point.x - node.point.x;
	const int deltaY = goalNode.point.y - node.point.y;
	const int deltaZ = goalNode.point.z - node.point.z;

	const int square = deltaX * deltaX + deltaY * deltaY + deltaZ * deltaZ;
	if (square < numCachedSquareRoots) {
		return squareRootCache[square];
	}
	return square;
}

static inline double getScoreOrInfinity(
    const std::unordered_map<unsigned int, double>& scores,
    const unsigned int nodeId) {
	const auto search = scores.find(nodeId);
	if (search != scores.end()) {
		return search->second;
	} else {
		return std::numeric_limits<double>::infinity();
	}
}

static inline std::vector<unsigned int> reconstructPath(
    std::unordered_map<unsigned int, unsigned int>& cameFrom,
    unsigned int currentNodeId) {
	std::vector<unsigned int> path{currentNodeId};
	while (true) {
		const auto search = cameFrom.find(currentNodeId);
		if (search == cameFrom.end()) {
			break;
		}

		currentNodeId = search->second;
		path.insert(path.begin(), currentNodeId);
	}
	return path;
}

// https://en.wikipedia.org/wiki/A*_search_algorithm
sol::object PointGraph::findShortestPath(unsigned int startNodeId,
                                         unsigned int goalNodeId,
                                         sol::this_state s) const {
	sol::state_view lua(s);

	const Node& goalNode = nodes.at(goalNodeId);

	// For node n, gScores[n] is the cost of the cheapest path from start to n
	// currently known.
	std::unordered_map<unsigned int, double> gScores;
	gScores.insert({startNodeId, 0.});

	auto compare = [&gScores](unsigned int a, unsigned int b) -> bool {
		return gScores.at(a) > gScores.at(b);
	};

	// The set of discovered nodes that may need to be (re-)expanded.
	// Initially, only the start node is known.
	std::priority_queue<unsigned int, std::vector<unsigned int>,
	                    decltype(compare)>
	    openSet(compare);

	openSet.push(startNodeId);

	// For node n, cameFrom[n] is the node immediately preceding it on the
	// cheapest path from start to n currently known.
	std::unordered_map<unsigned int, unsigned int> cameFrom;

	// For node n, fScores[n] := gScores[n] + h(n). fScores[n] represents our
	// current best guess as to how short a path from start to finish can be if it
	// goes through n.
	std::unordered_map<unsigned int, double> fScores;
	fScores.insert(
	    {startNodeId, getHeuristicScore(nodes.at(startNodeId), goalNode,
	                                    numCachedSquareRoots, squareRootCache)});

	std::unordered_set<unsigned int> hasVisited;

	while (!openSet.empty()) {
		const unsigned int currentNodeId = openSet.top();
		if (currentNodeId == goalNodeId) {
			const auto path = reconstructPath(cameFrom, currentNodeId);
			return sol::make_object(lua, sol::as_table(path));
		}
		openSet.pop();

		if (hasVisited.count(currentNodeId)) {
			continue;
		}

		hasVisited.insert(currentNodeId);

		const Node& currentNode = nodes.at(currentNodeId);
		for (const Link& link : currentNode.links) {
			const double tentativeGScore = gScores.at(currentNodeId) + link.cost;
			const double gScore = getScoreOrInfinity(gScores, link.toId);
			if (tentativeGScore < gScore) {
				// This path to neighbor is better than any previous one. Record it!
				cameFrom.insert({link.toId, currentNodeId});
				gScores.insert({link.toId, tentativeGScore});
				fScores.insert(
				    {link.toId,
				     tentativeGScore +
				         getHeuristicScore(nodes.at(link.toId), currentNode,
				                           numCachedSquareRoots, squareRootCache)});
				openSet.push(link.toId);
			}
		}
	}

	return sol::make_object(lua, sol::nil);
}