#include "../TP_2023192003/TP_2023192003/appearance_row.hpp"
#include <cassert>
#include <climits>
#include <cstdio>
int main() {
    using avatar_appearance::Value;
    const std::array<std::int32_t,5> good{{123,1,4,2,2}};
    Value value{}; assert(appearance_storage::decodeRow(123,good,value) && value==Value({1,4,2,2}));
    for(int index=0;index<5;++index) for(int invalid : {-1,INT_MAX}) {
        auto bad=good; bad[index]=invalid; const auto before=value;
        assert(!appearance_storage::decodeRow(123,bad,value) && value==before);
    }
    for(int index=1;index<5;++index) {
        auto bad=good; const int counts[]={0,HAMSTER_CHARACTER_KINDS,HAMSTER_FUR_COLORS,HAMSTER_EYE_STYLES,HAMSTER_SCARF_STYLES};
        bad[index]=counts[index]; const auto before=value;
        assert(!appearance_storage::decodeRow(123,bad,value) && value==before);
    }
    assert(!appearance_storage::decodeRow(124,good,value));
    assert(!appearance_storage::decodeRow(-1,good,value));
    std::puts("appearance DB row: account identity/ranges/no narrowing/invalid output preservation PASS");
}
