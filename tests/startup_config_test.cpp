#include "../TP_2023192003/TP_2023192003/startup_config.hpp"
#include <cassert>
#include <initializer_list>
int main() {
    int count = -1;
    assert(startup_config::monsterCount(nullptr,count) && count == 2000);
    assert(startup_config::monsterCount("0",count) && count == 0);
    assert(startup_config::monsterCount("200000",count) && count == 200000);
    for (auto value : {"", "-1", "200001", "99999999999999999", "1x", " 10", "+10"}) {
        count = 123;
        assert(!startup_config::monsterCount(value,count) && count == 123);
    }
}
