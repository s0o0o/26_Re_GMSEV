#include "../TP_2023192003/TP_2023192003/pathfinder.hpp"
#include "../TP_2023192003/TP_2023192003/world_map.hpp"
#include <cassert>
#include <cstdlib>

int main() {
    assert(findPathAStar(-1, 0, 0, 0).empty());
    auto same = findPathAStar(0, 0, 0, 0);
    assert(same.size() == 1);
    map_data[0][1] = 3;
    auto path = findPathAStar(0, 0, 2, 0);
    assert(path.size() == 5);
    assert(path.front() == std::make_pair(0, 0));
    assert(path.back() == std::make_pair(2, 0));
    for (std::size_t i = 1; i < path.size(); ++i) {
        assert(map_data[path[i].second][path[i].first] != 3);
        assert(std::abs(path[i].first - path[i-1].first) +
               std::abs(path[i].second - path[i-1].second) == 1);
    }
    assert(findPathAStar(0, 0, 1, 0).empty());
    map_data[1][0] = 3;
    assert(findPathAStar(0, 0, 2, 0).empty());
}
