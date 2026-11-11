#pragma once
#include "protocol.hpp"
#include "world_layout.hpp"
#include <array>
namespace town_npcs {
// Existing monType 0=player, 1/2=hostile. Reserve a distinct range for residents.
constexpr int firstType=100,baseId=MAX_USER+200000;
struct Resident { int id,type,x,y; const char* name; const char* asset; };
constexpr std::array<Resident,3> residents={{{baseId,100,12,9,"Boss","boss"},
    {baseId+1,101,12,15,"Pashmina","pashmina"},{baseId+2,102,14,14,"Snoozer","snoozer"}}};
constexpr bool isResident(int type) { return type>=firstType && type<firstType+static_cast<int>(residents.size()); }
constexpr bool hostile(int type) { return type==1 || type==2; }
inline const Resident* byType(int type) { return isResident(type) ? &residents[type-firstType] : nullptr; }
}
