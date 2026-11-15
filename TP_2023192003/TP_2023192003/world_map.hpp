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
// Canonical runtime APIs; legacy array addressing remains inside this boundary.
bool playerCanMove(world_regions::Position position) noexcept;
int mapTileAt(world_regions::Position position);
bool collectSeed(world_regions::Position position,respawn_policy::Tick now=respawn_policy::now());
