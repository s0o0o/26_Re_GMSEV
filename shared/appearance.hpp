#pragma once
#include <cstdint>

constexpr std::uint8_t HAMSTER_FUR_COLORS = 5;
constexpr bool validFurColor(std::uint8_t value) { return value < HAMSTER_FUR_COLORS; }
