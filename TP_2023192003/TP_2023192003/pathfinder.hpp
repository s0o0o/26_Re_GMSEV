#pragma once
#include <utility>
#include <vector>
#include "../../shared/world_regions.hpp"

// Returns start through goal, or an empty path if unreachable/invalid.
std::vector<std::pair<int, int>> findPathAStar(int startX, int startY, int goalX, int goalY);
std::vector<std::pair<int,int>> findPathAStar(world_regions::Position start,world_regions::Position goal);
