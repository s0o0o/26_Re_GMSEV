#pragma once
#include <algorithm>

namespace display_layout {
constexpr float width=900.f,height=800.f;
struct Fit {
    float left=0,top=0,width=0,height=0,scale=0;
    bool contains(float x,float y) const {
        return scale>0 && x>=left && y>=top && x<left+width && y<top+height;
    }
};
inline Fit fit(unsigned w,unsigned h) {
    if(!w || !h) return {};
    const float scale=(std::min)(w/width,h/height);
    const float fittedWidth=(std::min)(static_cast<float>(w),width*scale);
    const float fittedHeight=(std::min)(static_cast<float>(h),height*scale);
    return {(w-fittedWidth)/2,(h-fittedHeight)/2,fittedWidth,fittedHeight,scale};
}
}
