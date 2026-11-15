#include "../TP_2023192003/TP_2023192003/pathfinder.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <cassert>
#include <cstdlib>

int main() {
    assert(findPathAStar(-1, 0, 0, 0).empty());
    initializeFieldMap();
    auto same = findPathAStar(38, 7, 38, 7);
    assert(same.size() == 1);
    // The authored starter wall at local x=9..13,y=5 forces a detour.
    auto path = findPathAStar(40, 5, 46, 5);
    assert(path.size() == 9);
    assert(path.front() == std::make_pair(40, 5));
    assert(path.back() == std::make_pair(46, 5));
    for (std::size_t i = 1; i < path.size(); ++i) {
        assert(playerCanMove(path[i].first,path[i].second));
        assert(std::abs(path[i].first - path[i-1].first) +
               std::abs(path[i].second - path[i-1].second) == 1);
    }
    assert(findPathAStar(40, 5, 41, 5).empty());
    assert(findPathAStar(0, 0, 2, 0).empty()); // Actual walls, not an uninitialized zero-filled world.
}
