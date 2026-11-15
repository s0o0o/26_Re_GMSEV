#pragma once
#include "load_test_config.hpp"
#include <string>
inline bool approvedGuiRange(const char* confirmation,const char* start,int count,int& accountStart) {
    if(!confirmation || confirmation[0]!='1' || confirmation[1]!=0 || count<1 || count>300 || !start) return false;
    const std::string value(start);
    if(value=="900001000") accountStart=900001000;
    else if(value=="900000000") accountStart=900000000;
    else return false;
    return true;
}
