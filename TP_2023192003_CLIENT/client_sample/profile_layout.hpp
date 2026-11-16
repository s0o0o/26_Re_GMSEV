#pragma once
#include "ui_alignment.hpp"
namespace profile_layout {
// The legacy profile raster owns NAME and SEED labels. Keep the dynamic name
// inside its own slot; coordinates are in the existing 900x800 logical view.
inline sf::FloatRect nameSlot() { return {242,678,128,31}; }
inline void placeName(sf::Text& text) { centerText(text,nameSlot()); }
}
