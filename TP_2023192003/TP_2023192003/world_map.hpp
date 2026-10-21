#pragma once

#include "server_config.hpp"

extern int map_data[MAP_HEIGHT][MAP_WIDTH];

bool playerCanMove(int x, int y) noexcept;
int mapTileAt(int x,int y);
bool collectSeed(int x,int y);
void initializeFieldMap();
