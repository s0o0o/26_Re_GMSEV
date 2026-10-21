#pragma once
#include <utility>
#include <vector>

// Returns start through goal, or an empty path if unreachable/invalid.
std::vector<std::pair<int, int>> findPathAStar(int startX, int startY, int goalX, int goalY);
