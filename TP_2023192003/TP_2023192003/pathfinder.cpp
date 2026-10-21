#include "pathfinder.hpp"
#include "world_map.hpp"
#include "server_config.hpp"
#include <algorithm>
#include <cstdlib>
#include <memory>
#include <queue>
#include <unordered_set>

namespace {
struct Node {
	int x, y;
	int gCost, hCost;
	Node* parent;

	Node(int x_, int y_, int g_, int h_, Node* p)
		: x(x_), y(y_), gCost(g_), hCost(h_), parent(p) {}

	int fCost() const { return gCost + hCost; }
	bool operator>(const Node& other) const { return fCost() > other.fCost(); }
};

struct NodeComp {
	bool operator()(const Node* a, const Node* b) const {
		return a->fCost() > b->fCost();
	}
};

bool isValid(int x, int y) {
	return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT && map_data[y][x] != 3;
}

int heuristic(int x1, int y1, int x2, int y2) {	// ÈÞ¸®½ºÆ½
	return abs(x1 - x2) + abs(y1 - y2);
}

} // namespace

std::vector<std::pair<int, int>> findPathAStar(int startX, int startY, int goalX, int goalY) {
	if (!isValid(startX, startY) || !isValid(goalX, goalY)) return {};
	std::vector<std::unique_ptr<Node>> nodes;
	std::priority_queue<Node*, std::vector<Node*>, NodeComp> openList;
	std::unordered_set<long long> closedSet;

	auto hash = [](int x, int y) -> long long { return ((long long)y << 32) | x; };

	nodes.push_back(std::make_unique<Node>(startX, startY, 0, heuristic(startX, startY, goalX, goalY), nullptr));
	Node* start = nodes.back().get();
	openList.push(start);

	while (!openList.empty()) {
		Node* current = openList.top();
		openList.pop();

		if (current->x == goalX && current->y == goalY) {
			std::vector<std::pair<int, int>> path;
			while (current) {
				path.emplace_back(current->x, current->y);
				current = current->parent;
			}
			std::reverse(path.begin(), path.end());
			return path;
		}

		if (!closedSet.insert(hash(current->x, current->y)).second) continue;

		const int dx[4] = { 0, 0, -1, 1 };
		const int dy[4] = { -1, 1, 0, 0 };

		for (int dir = 0; dir < 4; ++dir) {
			int nx = current->x + dx[dir];
			int ny = current->y + dy[dir];

			if (!isValid(nx, ny)) continue;
			if (closedSet.count(hash(nx, ny))) continue;

			int gCost = current->gCost + 1;
			int hCost = heuristic(nx, ny, goalX, goalY);
			nodes.push_back(std::make_unique<Node>(nx, ny, gCost, hCost, current));
			Node* neighbor = nodes.back().get();
			openList.push(neighbor);
		}
	}
	return {};
}
