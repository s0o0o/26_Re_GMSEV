#include "../TP_2023192003_CLIENT/client_sample/display_layout.hpp"
#include <cassert>
#include <cmath>
int main() {
    for(auto size : {std::pair<unsigned,unsigned>{900,800},{1280,720},{1920,1080},{2560,1440},{3440,1440},{600,1000},{320,240},{1,1}}) {
        const auto fit=display_layout::fit(size.first,size.second);
        assert(fit.scale>0 && std::abs(fit.width/fit.height-900.f/800)<0.001f);
        assert(fit.left>=0 && fit.top>=0 && fit.width<=size.first+0.01f && fit.height<=size.second+0.01f);
        const auto x=fit.left+595*fit.scale,y=fit.top+636*fit.scale;
        assert(fit.contains(x,y));
        assert(std::abs((x-fit.left)/fit.scale-595)<0.001f);
        assert(!fit.contains(-1,0) && !fit.contains(size.first,size.second));
        if(fit.left>1) assert(!fit.contains(0,size.second/2.f));
    }
    assert(!display_layout::fit(0,800).contains(0,0));
    assert(!display_layout::fit(900,0).contains(0,0));
}
