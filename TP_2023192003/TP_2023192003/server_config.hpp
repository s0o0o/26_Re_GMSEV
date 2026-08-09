#pragma once

#include "..\..\shared\protocol.hpp"

constexpr int NUM_MONSTER = 200000;

constexpr int MAP_HEIGHT = 2000;
constexpr int MAP_WIDTH = 2000;

constexpr int VIEW_RANGE = 7;
constexpr int SECTOR_SIZE = 20;
constexpr int MAX_SECTOR_X = (MAP_WIDTH + SECTOR_SIZE - 1) / SECTOR_SIZE;
constexpr int MAX_SECTOR_Y = (MAP_HEIGHT + SECTOR_SIZE - 1) / SECTOR_SIZE;
constexpr int TIMER_THREAD_COUNT = 8;
