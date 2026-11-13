#pragma once

#include "..\..\shared\protocol.hpp"
#include "../../shared/world_layout.hpp"

constexpr int NUM_MONSTER = 200000;

constexpr int MAP_HEIGHT = world_layout::height;
constexpr int MAP_WIDTH = world_layout::width;

constexpr int VIEW_RANGE = 7;
constexpr int SECTOR_SIZE = 20;
constexpr int TIMER_THREAD_COUNT = 8;
