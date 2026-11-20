#include "../shared/resident_quest_map.hpp"
#include <cassert>
int main(){using namespace resident_quest_log;assert(resident_quest_map::marker({3,3,101,GoalKind::Inspect})==3);assert(resident_quest_map::marker({3,4,101,GoalKind::Experience})==4);assert(!resident_quest_map::marker({3,4,101,GoalKind::Report}));assert(!resident_quest_map::marker({2,99,100,GoalKind::Inspect}));assert(!resident_quest_map::marker({}));}
