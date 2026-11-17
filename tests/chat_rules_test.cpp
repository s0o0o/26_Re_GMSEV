#include "../shared/chat_rules.hpp"
#include <cassert>
#include <cstring>
int main(){
    using namespace chat_rules;
    assert(valid("hello") && valid(u8"안녕 친구"));assert(!valid("") && !valid("  ") && !valid("a\nb"));
    assert(!valid(std::string(120,'x')) && valid(std::string(119,'x')));
    std::string korean;for(int i=0;i<39;++i)korean+=u8"가";assert(valid(korean) && valid(korean+"ab"));assert(!valid(korean+u8"가"));
    for(auto s:{"\xc0\x80","\xed\xa0\x80","\xf4\x90\x80\x80","\xe3\x81","\x80"})assert(!valid(s));
    std::string s=u8"가나다";eraseLast(s);assert(s==u8"가나");eraseLast(s);eraseLast(s);eraseLast(s);assert(s.empty());
    char raw[120]{};std::string out;strcpy_s(raw,u8"안녕");assert(decode(raw,out) && out==u8"안녕");
    memset(raw,'a',sizeof(raw));assert(!decode(raw,out));
    std::uint64_t next=0;assert(admit(0,next));assert(!admit(499,next));assert(admit(500,next));
}
