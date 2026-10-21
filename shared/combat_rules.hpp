#pragma once
#include <cstdint>
enum class AttackFeedback : std::uint8_t { NoSeeds=1, NoTarget=2, SafeZone=3 };
constexpr bool validAttackFeedback(std::uint8_t value) { return value >= 1 && value <= 3; }
namespace combat_rules {
struct Hit { bool applied; bool died; int damage; };
template<class Hp,class Seeds>
Hit apply(Hp& hp,Seeds& seeds,int damage) {
    if (hp <= 0 || seeds <= 0 || damage <= 0) return {false,false,0};
    const int actual=damage < hp ? damage : hp;
    hp=static_cast<Hp>(hp-actual); --seeds;
    return {true,hp == 0,actual};
}
}
