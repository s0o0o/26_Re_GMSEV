#pragma once
#include <cstdint>
#include <cstddef>

constexpr std::uint8_t HAMSTER_FUR_COLORS = 5;
constexpr bool validFurColor(std::uint8_t value) { return value < HAMSTER_FUR_COLORS; }
constexpr std::uint8_t HAMSTER_EYE_STYLES = 3;
constexpr std::uint8_t HAMSTER_SCARF_STYLES = 3;
constexpr bool validEyeStyle(std::uint8_t value) { return value < HAMSTER_EYE_STYLES; }
constexpr bool validScarfStyle(std::uint8_t value) { return value < HAMSTER_SCARF_STYLES; }
constexpr std::size_t HAMSTER_APPEARANCES = HAMSTER_FUR_COLORS * HAMSTER_EYE_STYLES * HAMSTER_SCARF_STYLES;
constexpr std::size_t appearanceIndex(std::uint8_t fur, std::uint8_t eyes, std::uint8_t scarf) {
    return (fur * HAMSTER_EYE_STYLES + eyes) * HAMSTER_SCARF_STYLES + scarf;
}
