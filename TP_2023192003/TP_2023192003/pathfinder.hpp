#pragma once
#include <utility>
#include <vector>
#include <cstddef>
#include "../../shared/world_regions.hpp"

// Returns start through goal, or an empty path if unreachable/invalid.
std::vector<std::pair<int, int>> findPathAStar(int startX, int startY, int goalX, int goalY);
struct PathSearchStats { std::size_t expanded=0,allocated=0; bool exhausted=false; };
std::vector<std::pair<int,int>> findPathAStar(world_regions::Position start,world_regions::Position goal,
    std::size_t maxExpanded=1024,PathSearchStats* stats=nullptr);
