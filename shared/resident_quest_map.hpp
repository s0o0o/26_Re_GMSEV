#pragma once
#include "resident_quest_log.hpp"
namespace resident_quest_map {
inline int marker(resident_quest_log::Goal g){using K=resident_quest_log::GoalKind;return g.questId && (g.kind==K::Visit || g.kind==K::Inspect || g.kind==K::Experience) && field_investigation::byId(g.target)?g.target:0;}
}
