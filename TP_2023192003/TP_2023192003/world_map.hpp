#pragma once

#include "server_config.hpp"
#include "../../shared/respawn_policy.hpp"
#include "../../shared/field_content.hpp"
#include <vector>

extern int map_data[MAP_HEIGHT][MAP_WIDTH];

bool playerCanMove(int x, int y) noexcept;
int mapTileAt(int x,int y);
bool collectSeed(int x,int y,respawn_policy::Tick now=respawn_policy::now());
std::vector<field_content::Point> restoreSeeds(respawn_policy::Tick now);
void initializeFieldMap();
