#pragma once
#include "../../shared/appearance.hpp"
#include <array>
namespace appearance_storage {
inline bool decodeRow(int account,const std::array<std::int32_t,5>& row,avatar_appearance::Value& output) {
    if(account<0 || row[0]!=account || row[1]<0 || row[1]>=HAMSTER_CHARACTER_KINDS
        || row[2]<0 || row[2]>=HAMSTER_FUR_COLORS || row[3]<0 || row[3]>=HAMSTER_EYE_STYLES
        || row[4]<0 || row[4]>=HAMSTER_SCARF_STYLES) return false;
    output={static_cast<std::uint8_t>(row[1]),static_cast<std::uint8_t>(row[2]),
        static_cast<std::uint8_t>(row[3]),static_cast<std::uint8_t>(row[4])};
    return true;
}
}
