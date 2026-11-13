#pragma once
#include <array>
#include <cstdint>
#include <climits>
#include <string>

namespace database_contract {
constexpr std::size_t count=9;
using Values=std::array<std::int32_t,count>;
using Lengths=std::array<std::int64_t,count>;
struct Column { std::string name; bool int32=false; };
enum class Fetch { Row,End,Error };
enum class Result { Found,NotFound,Error };
inline const char* column(std::size_t index) {
    static const char* names[]={"USER_ID","USER_X","USER_Y","USER_HP","USER_LEVEL","USER_EXP","USER_SEED_NUM","USER_MAX_HP","USER_DAMAGE"};
    return index<count ? names[index] : "";
}
inline bool matchesColumn(std::size_t index,const std::string& name,bool int32) {
    if(index>=count || !int32) return false;
    std::string normalized=name;
    for(char& c : normalized) if(c>='a' && c<='z') c=static_cast<char>(c-'a'+'A');
    return normalized==column(index);
}
// Validate the complete row before exposing anything to the caller. Length is SQL_C_LONG's 4 bytes.
template<class Length>
bool acceptRow(std::int32_t requested,const Values& input,const std::array<Length,count>& lengths,Values& output) {
    if(requested<0 || input[0]!=requested) return false;
    for(auto length : lengths) if(length!=static_cast<Length>(sizeof(std::int32_t))) return false;
    // Coordinates are validated/recovered by the position codec before short conversion.
    for(std::size_t i=3;i<count;++i) if(input[i]<0 || input[i]>SHRT_MAX) return false;
    if(input[7]==0 || input[3]>input[7]) return false;
    output=input; return true;
}
template<class Reader>
Result readSingle(Reader& reader,std::int32_t requested,Values& output) {
    if(requested<0) return Result::Error;
    std::array<Column,count> columns;
    if(!reader.describe(columns)) return Result::Error;
    for(std::size_t i=0;i<count;++i) if(!matchesColumn(i,columns[i].name,columns[i].int32)) return Result::Error;
    Values input{},accepted{}; Lengths lengths{};
    const auto fetched=reader.fetch(input,lengths);
    if(fetched==Fetch::End) return Result::NotFound;
    if(fetched!=Fetch::Row || !acceptRow(requested,input,lengths,accepted)) return Result::Error;
    if(reader.fetch(input,lengths)!=Fetch::End) return Result::Error;
    output=accepted; return Result::Found;
}
}
