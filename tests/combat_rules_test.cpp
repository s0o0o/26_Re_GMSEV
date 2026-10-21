#include "../shared/combat_rules.hpp"
#include <cassert>
#include <mutex>
#include <thread>
#include <vector>
int main() {
    short hp=5,seeds=2;
    auto hit=combat_rules::apply(hp,seeds,2);
    assert(hit.applied && !hit.died && hit.damage == 2 && hp == 3 && seeds == 1);
    hit=combat_rules::apply(hp,seeds,99);
    assert(hit.applied && hit.died && hit.damage == 3 && hp == 0 && seeds == 0);
    seeds=10;
    assert(!combat_rules::apply(hp,seeds,1).applied && seeds == 10);
    hp=5; seeds=0;
    assert(!combat_rules::apply(hp,seeds,1).applied && hp == 5);
    seeds=3;
    assert(!combat_rules::apply(hp,seeds,0).applied && hp == 5 && seeds == 3);
    std::mutex guard;
    int deaths=0;
    hp=1; seeds=10;
    std::vector<std::thread> attackers;
    for(int i=0;i<16;++i) attackers.emplace_back([&] {
        std::lock_guard<std::mutex> lock(guard);
        if(combat_rules::apply(hp,seeds,1).died) ++deaths;
    });
    for(auto& attacker : attackers) attacker.join();
    assert(deaths == 1 && seeds == 9);
}
