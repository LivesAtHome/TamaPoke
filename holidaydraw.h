// Holiday add-on: the decorations, drawn into the scene after the weather and
// before the creature. Include after gfx, C565, UI_WHITE, HORIZON and LCD_WIDTH exist.
// Layout keeps clear of the header text (y < 110), the poop corner (bottom left)
// and the bottom panel (y >= 312, painted over everything anyway).
#pragma once
#include "holiday.h"

static inline uint16_t holMix(uint16_t a, uint16_t b, int i, int n) {   // same idea as lerp565
  int ar = a >> 11, ag = (a >> 5) & 63, ab = a & 31, br = b >> 11, bg = (b >> 5) & 63, bb = b & 31;
  return (uint16_t)(((ar + (br - ar) * i / n) << 11) | ((ag + (bg - ag) * i / n) << 5) | (ab + (bb - ab) * i / n));
}

// a sagging string across the upper sky; calls f(i, x, y) for n evenly spaced points
template <typename F>
static inline void holGarland(int n, F f) {
  const int x0 = 34, x1 = LCD_WIDTH - 34, top = 118, sag = 26;
  int px = x0, py = top;
  for (int i = 0; i <= 40; i++) {               // the string itself
    float t = i / 40.0f;
    int x = x0 + (int)((x1 - x0) * t), y = top + (int)(sag * 4 * t * (1 - t));
    if (i) gfx->drawLine(px, py, x, y, C565(0x30, 0x30, 0x38));
    px = x; py = y;
  }
  for (int i = 0; i < n; i++) {
    float t = (i + 0.5f) / n;
    f(i, x0 + (int)((x1 - x0) * t), top + (int)(sag * 4 * t * (1 - t)));
  }
}

static void holFirework(uint32_t now, int slot, int cx, int cy, const uint16_t *cols) {
  const uint32_t period = 1700;
  uint32_t t = (now + slot * 850) % period;
  if (t > 1100) return;                          // a pause between bursts
  int r = 6 + (int)(t * 46 / 1100);
  uint16_t c = cols[((now + slot * 850) / period + slot) % 3];
  for (int k = 0; k < 12; k++) {
    float a = k * 0.5236f;
    int x = cx + (int)(cosf(a) * r), y = cy + (int)(sinf(a) * r) + (int)(t * t / 60000);   // a little droop
    gfx->fillCircle(x, y, t < 800 ? 3 : 2, c);
  }
  if (t < 200) gfx->fillCircle(cx, cy, 4, UI_WHITE);   // the bang
}

static void holHeart(int x, int y, int s, uint16_t c) {
  gfx->fillCircle(x - s, y, s, c);
  gfx->fillCircle(x + s, y, s, c);
  gfx->fillTriangle(x - 2 * s, y + s / 3, x + 2 * s, y + s / 3, x, y + 2 * s + s / 2, c);
}

static void holShamrock(int x, int y) {
  const uint16_t g = C565(0x2e, 0x9e, 0x48), dk = C565(0x1c, 0x6c, 0x30);
  gfx->drawLine(x, y, x + 3, y + 12, dk);
  gfx->fillCircle(x - 5, y - 2, 5, g);
  gfx->fillCircle(x + 5, y - 2, 5, g);
  gfx->fillCircle(x, y - 9, 5, g);
}

static void holEgg(int x, int y, uint16_t body, uint16_t band) {
  gfx->fillCircle(x, y, 11, body);
  gfx->fillCircle(x, y - 7, 8, body);
  gfx->fillRect(x - 10, y - 4, 21, 4, band);
  gfx->fillRect(x - 9, y + 4, 19, 2, UI_WHITE);
}

static void holPumpkin(int x, int y, bool glow) {
  const uint16_t o = C565(0xf0, 0x84, 0x1c), od = C565(0xc8, 0x64, 0x10);
  gfx->fillCircle(x - 9, y, 13, od);
  gfx->fillCircle(x + 9, y, 13, od);
  gfx->fillCircle(x, y, 15, o);
  gfx->fillRect(x - 2, y - 20, 5, 7, C565(0x4c, 0x7a, 0x2a));   // stem
  uint16_t f = glow ? C565(0xff, 0xe0, 0x50) : C565(0x30, 0x20, 0x10);
  gfx->fillTriangle(x - 9, y - 1, x - 3, y - 1, x - 6, y - 7, f);   // eyes
  gfx->fillTriangle(x + 3, y - 1, x + 9, y - 1, x + 6, y - 7, f);
  gfx->fillTriangle(x - 8, y + 4, x + 8, y + 4, x, y + 9, f);       // grin
}

static void holBat(int x, int y, bool up) {
  const uint16_t b = C565(0x5a, 0x48, 0x70);   // dusky purple: visible against the night sky
  gfx->fillCircle(x, y, 3, b);
  int wy = up ? -6 : 4;
  gfx->fillTriangle(x - 2, y, x - 14, y + wy, x - 8, y + 2, b);
  gfx->fillTriangle(x + 2, y, x + 14, y + wy, x + 8, y + 2, b);
}

static void holLeaf(int x, int y, int s, uint16_t c) {
  gfx->fillTriangle(x - s, y, x + s, y, x, y - s - 2, c);
  gfx->fillTriangle(x - s, y, x + s, y, x, y + s, c);
}

static void holTree(int x, int y, uint32_t now) {        // y = ground under the trunk
  const uint16_t g = C565(0x1e, 0x7a, 0x3c);
  gfx->fillRect(x - 4, y - 10, 8, 10, C565(0x6a, 0x40, 0x20));
  gfx->fillTriangle(x - 26, y - 10, x + 26, y - 10, x, y - 40, g);
  gfx->fillTriangle(x - 21, y - 28, x + 21, y - 28, x, y - 56, g);
  gfx->fillTriangle(x - 15, y - 44, x + 15, y - 44, x, y - 68, g);
  static const int8_t BX[6] = { -14, 10, -6, 12, -9, 4 }, BY[6] = { 16, 18, 32, 34, 48, 54 };
  static const uint16_t BC[3] = { 0xF800, 0xFFE0, 0x001F };
  for (int i = 0; i < 6; i++)
    gfx->fillCircle(x + BX[i], y - BY[i], 3, ((now / 500 + i) % 4) ? BC[i % 3] : UI_WHITE);
  gfx->fillTriangle(x - 6, y - 66, x + 6, y - 66, x, y - 76, C565(0xff, 0xd9, 0x40));   // star
  gfx->fillTriangle(x - 6, y - 72, x + 6, y - 72, x, y - 62, C565(0xff, 0xd9, 0x40));
}

void drawHoliday(uint8_t hol, uint32_t now, bool night) {
  switch (hol) {
    case HOL_NEWYEAR: {
      static const uint16_t C[3] = { C565(0xff, 0xd9, 0x40), C565(0xff, 0x6a, 0xb0), C565(0x7a, 0xe0, 0xff) };
      if (night) {
        holFirework(now, 0, 140, 170, C);
        holFirework(now, 1, 330, 160, C);
      } else {
        for (int k = 0; k < 36; k++) {           // confetti
          int x = (int)((k * 127 + (int)(sinf(now / 700.0f + k) * 10)) % LCD_WIDTH);
          int y = (int)((k * 97 + now / 20) % 300) + 10;
          gfx->fillRect(x, y, 4, 3, C[k % 3]);
        }
      }
      break;
    }
    case HOL_VALENTINE:
      for (int k = 0; k < 6; k++) {               // hearts drifting up from the ground
        int y = 300 - (int)((now / 30 + k * 53) % 190);
        int x = 60 + k * 66 + (int)(sinf(now / 900.0f + k) * 10);
        holHeart(x, y, 4 + (k % 2), k % 2 ? C565(0xe8, 0x40, 0x6a) : C565(0xff, 0x8a, 0xb0));
      }
      break;
    case HOL_STPATRICK:
      holShamrock(250, 300);
      holShamrock(318, 284);
      holShamrock(380, 302);
      holShamrock(428, 280);
      break;
    case HOL_EASTER:
      holEgg(334, 290, C565(0xff, 0xb0, 0xd0), C565(0x9a, 0x70, 0xe0));
      holEgg(378, 298, C565(0xb0, 0xe8, 0xff), C565(0xff, 0xd0, 0x50));
      holEgg(420, 284, C565(0xd8, 0xf5, 0xa0), C565(0xff, 0x80, 0x80));
      break;
    case HOL_JULY4: {
      static const uint16_t RWB[3] = { C565(0xe0, 0x2c, 0x2c), UI_WHITE, C565(0x2a, 0x4c, 0xc8) };
      holGarland(11, [](int i, int x, int y) { gfx->fillTriangle(x - 10, y, x + 10, y, x, y + 16, RWB[i % 3]); });
      if (night) {
        holFirework(now, 0, 130, 185, RWB);
        holFirework(now, 1, 320, 175, RWB);
      }
      break;
    }
    case HOL_HALLOWEEN:
      holPumpkin(368, 284, night);
      holPumpkin(420, 296, night);
      if (night)
        for (int k = 0; k < 3; k++) {
          int x = (int)((now / (14 + k * 4) + k * 160) % (LCD_WIDTH + 60)) - 30;
          int y = 140 + k * 26 + (int)(sinf(now / 400.0f + k) * 8);
          holBat(x, y, ((now / 140 + k) & 1) != 0);
        }
      break;
    case HOL_THANKSGIVING: {
      static const uint16_t L[3] = { C565(0xe8, 0x7a, 0x1e), C565(0xc0, 0x3c, 0x22), C565(0x9a, 0x6a, 0x2c) };
      for (int k = 0; k < 14; k++) {              // leaves drifting down
        uint32_t hx = (uint32_t)k * 2654435761u;      // scatter them, not a neat diagonal
        int x = (int)(((hx >> 7) % LCD_WIDTH + (int)(sinf(now / 1100.0f + k) * 18) + LCD_WIDTH) % LCD_WIDTH);
        int y = (int)(((hx >> 17) % 300 + now / 40) % 300) + 10;
        holLeaf(x, y, 5 + (k % 3), L[k % 3]);
      }
      break;
    }
    case HOL_CHRISTMAS: {
      static const uint16_t B[4] = { 0xF800, C565(0x30, 0xd0, 0x50), C565(0xff, 0xd9, 0x40), C565(0x50, 0x90, 0xff) };
      holGarland(14, [now, night](int i, int x, int y) {
        bool on = ((now / 600 + i) % 3) != 0 || !night;
        uint16_t c = B[i % 4];
        gfx->fillRect(x - 1, y, 3, 3, C565(0x30, 0x30, 0x38));
        gfx->fillCircle(x, y + 6, 4, on ? c : holMix(c, 0, 1, 2));
      });
      holTree(412, 302, now);
      break;
    }
    default:
      break;
  }
}
