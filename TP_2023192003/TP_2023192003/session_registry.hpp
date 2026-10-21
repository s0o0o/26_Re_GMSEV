#pragma once

#include <memory>
#include <concurrent_unordered_map.h>
#include "game_session.hpp"

// Transitional shared registry. Ownership and lookup behavior are unchanged.
using SessionRegistry =
    concurrency::concurrent_unordered_map<long long, std::shared_ptr<GAMESESSION>>;

extern SessionRegistry clients;
