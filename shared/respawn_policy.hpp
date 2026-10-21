#pragma once
#include <chrono>
#include <cstdint>
namespace respawn_policy {
using Tick=std::uint64_t;
constexpr Tick seedDelay=20000, monsterDelay=30000;
inline Tick now() { return static_cast<Tick>(std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::steady_clock::now().time_since_epoch()).count()); }
constexpr bool due(Tick deadline,Tick time) { return deadline != 0 && time >= deadline; }
constexpr bool accepts(std::uint64_t ticket,std::uint64_t current,bool dead) { return !dead && ticket == current; }
}
