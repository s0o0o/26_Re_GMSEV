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

bool isValid(world_regions::Id region,int x, int y) {
	return playerCanMove({region,x,y});
}

int heuristic(int x1, int y1, int x2, int y2) {	// ÈÞ¸®½ºÆ½
	return abs(x1 - x2) + abs(y1 - y2);
}

} // namespace

std::vector<std::pair<int,int>> findPathAStar(int startX,int startY,int goalX,int goalY) {
    const auto start=world_regions::decode(startX,startY),goal=world_regions::decode(goalX,goalY);
    auto path=findPathAStar(start,goal);
    for(auto& point : path) point.first+=world_regions::describe(start.region).originX;
    return path;
}
std::vector<std::pair<int,int>> findPathAStar(world_regions::Position start,world_regions::Position goal,
    std::size_t maxExpanded,PathSearchStats* stats) {
    PathSearchStats local; if(!stats) stats=&local; *stats={};
    if(!maxExpanded) { stats->exhausted=true; return {}; }
    if(!world_regions::same(start,goal)) return {};
    const auto region=start.region;
    const int startX=start.x,startY=start.y,goalX=goal.x,goalY=goal.y;
	if (!isValid(region,startX,startY) || !isValid(region,goalX,goalY)) return {};
	std::vector<std::unique_ptr<Node>> nodes;
	std::priority_queue<Node*, std::vector<Node*>, NodeComp> openList;
	std::unordered_set<long long> closedSet;

	auto hash = [](int x, int y) -> long long { return ((long long)y << 32) | x; };

	nodes.push_back(std::make_unique<Node>(startX, startY, 0, heuristic(startX, startY, goalX, goalY), nullptr));
	Node* startNode = nodes.back().get();
	openList.push(startNode);

	while (!openList.empty()) {
        stats->allocated=nodes.size();
        if(stats->expanded>=maxExpanded) { stats->exhausted=true; return {}; }
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
        ++stats->expanded;

		const int dx[4] = { 0, 0, -1, 1 };
		const int dy[4] = { -1, 1, 0, 0 };

		for (int dir = 0; dir < 4; ++dir) {
			int nx = current->x + dx[dir];
			int ny = current->y + dy[dir];

			if (!isValid(region,nx,ny)) continue;
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
