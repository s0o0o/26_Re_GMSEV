#include "world_map.hpp"

int map_data[MAP_HEIGHT][MAP_WIDTH];

bool playerCanMove(int x, int y) noexcept
{
	if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT) {
		return false;
	}

	return map_data[y][x] != 3;
}
