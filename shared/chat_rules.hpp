#pragma once
#include <cstddef>
#include <cstdint>
#include <string>
namespace chat_rules {
constexpr std::size_t maxBytes=119;
inline bool valid(const std::string& s) {
    if(s.empty() || s.size()>maxBytes) return false;
    bool ink=false;
    for(std::size_t i=0;i<s.size();) {
        const auto a=static_cast<unsigned char>(s[i]);
        std::uint32_t cp=0;std::size_t n=0;
        if(a<0x80){cp=a;n=1;}
        else if(a>=0xc2 && a<=0xdf){cp=a&31;n=2;}
        else if(a>=0xe0 && a<=0xef){cp=a&15;n=3;}
        else if(a>=0xf0 && a<=0xf4){cp=a&7;n=4;}
        else return false;
        if(i+n>s.size()) return false;
        for(std::size_t j=1;j<n;++j){auto b=static_cast<unsigned char>(s[i+j]);if((b&0xc0)!=0x80)return false;cp=(cp<<6)|(b&63);}
        if((n==2 && cp<0x80)||(n==3 && cp<0x800)||(n==4 && cp<0x10000)||cp>0x10ffff||(cp>=0xd800 && cp<=0xdfff))return false;
        if(cp<32 || (cp>=127 && cp<=159) || cp==0x2028 || cp==0x2029) return false;
        if(cp!=32 && cp!=0x3000)ink=true;
        i+=n;
    }
    return ink;
}
template<std::size_t N> inline bool decode(const char (&raw)[N],std::string& out) {
    std::size_t n=0;while(n<N && raw[n])++n;
    if(n==N)return false;
    std::string value(raw,n);if(!valid(value))return false;out=std::move(value);return true;
}
inline void eraseLast(std::string& s) {if(s.empty())return;auto i=s.size()-1;while(i && (static_cast<unsigned char>(s[i])&0xc0)==0x80)--i;s.resize(i);}
inline bool admit(std::uint64_t now,std::uint64_t& next) {if(now<next)return false;next=now+500;return true;}
}
