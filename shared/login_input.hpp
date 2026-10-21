#pragma once
#include "protocol.hpp"

namespace login_input {
inline bool allowed(std::uint32_t c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
        (c >= '0' && c <= '9') || c == '_' ||
        (c >= 0xAC00 && c <= 0xD7A3) ||
        (c >= 0x1100 && c <= 0x11FF) || (c >= 0x3130 && c <= 0x318F);
}

inline bool accountId(const char* text, std::int32_t& result) {
    if (!text || !*text) return false;
    std::int32_t value = 0;
    for (; *text; ++text) {
        if (*text < '0' || *text > '9') return false;
        int digit = *text - '0';
        if (value > ((std::numeric_limits<std::int32_t>::max)() - digit) / 10) return false;
        value = value * 10 + digit;
    }
    result = value;
    return true;
}

// Bounded UTF-8 validation: require a terminator and reject overlong sequences.
inline bool nickname(const char* text, std::size_t capacity) {
    if (!text) return false;
    std::size_t i = 0, count = 0;
    while (i < capacity && text[i]) {
        unsigned char first = static_cast<unsigned char>(text[i++]);
        std::uint32_t cp = first;
        int extra = 0;
        std::uint32_t minimum = 0;
        if (first >= 0xC2 && first <= 0xDF) { extra = 1; cp = first & 0x1F; minimum = 0x80; }
        else if (first >= 0xE0 && first <= 0xEF) { extra = 2; cp = first & 0x0F; minimum = 0x800; }
        else if (first >= 0xF0 && first <= 0xF4) { extra = 3; cp = first & 7; minimum = 0x10000; }
        else if (first >= 0x80) return false;
        for (int j = 0; j < extra; ++j) {
            if (i >= capacity) return false;
            auto next = static_cast<unsigned char>(text[i++]);
            if ((next & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (next & 0x3F);
        }
        if (cp < minimum || !allowed(cp) || ++count > MAX_NICKNAME_CODEPOINTS) return false;
    }
    return i < capacity && count > 0;
}
}
