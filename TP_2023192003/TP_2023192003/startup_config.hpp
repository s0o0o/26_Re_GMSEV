#pragma once
#include <cstdlib>

namespace startup_config {
constexpr int defaultMonsters = 2000;
constexpr int maxMonsters = 200000;
inline bool monsterCount(const char* text, int& output) {
    if (!text) { output = defaultMonsters; return true; }
    if (!*text) return false;
    int value = 0;
    for (auto p=text; *p; ++p) {
        if (*p < '0' || *p > '9') return false;
        if (value > (maxMonsters-(*p-'0'))/10) return false;
        value = value*10+(*p-'0');
    }
    output = value;
    return true;
}
}
