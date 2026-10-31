#pragma once
#include <stddef.h>
#include <stdint.h>

constexpr int kPanelCanvasW = 792;
constexpr int kPanelCanvasH = 272;
constexpr int kPanelBytes = 800 / 8 * 272;  // driver framebuffer: 800x272 (8 dead columns), 1 = white

// Packs a 792x272 1-bit canvas (row-major, MSB first, 99 bytes per row, 1 = ink) into the Elecrow
// driver's framebuffer for Rotation 180: rows and columns flipped, and the right half shifted by
// 8 columns to skip the gap between the two SSD1683 controllers. Same mapping as the driver's
// Paint_SetPixel, without the per-pixel call overhead.
void packForPanel(const uint8_t *canvasBits, uint8_t *out);
