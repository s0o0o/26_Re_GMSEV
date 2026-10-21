#pragma once

#include "..\..\shared\protocol.hpp"
#include "../../shared/world_layout.hpp"

constexpr int NUM_MONSTER = 200000;

constexpr int MAP_HEIGHT = world_layout::height;
constexpr int MAP_WIDTH = world_layout::width;

constexpr int VIEW_RANGE = 7;
constexpr int SECTOR_SIZE = 20;
constexpr int MAX_SECTOR_X = (MAP_WIDTH + SECTOR_SIZE - 1) / SECTOR_SIZE;
constexpr int MAX_SECTOR_Y = (MAP_HEIGHT + SECTOR_SIZE - 1) / SECTOR_SIZE;
constexpr int TIMER_THREAD_COUNT = 8;
