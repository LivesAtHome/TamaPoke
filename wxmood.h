// Weather add-on: how the creature feels about the real weather outside, by type.
// Pure functions (no hardware) so they are easy to check on a desktop.
//
//   +1  it loves this weather   -> a cheerful line, and JOY drains half as fast
//   -1  it does not like it     -> a grumpy line only (never extra drain:
//                                  the weather alone can never cause a slip-up)
//    0  neutral                 -> sometimes a calm line, no effect
#pragma once
#include <stdint.h>
#include "weather.h"
#include "wxmood_tbl.h"

enum : uint8_t { WXA_NONE = 0, WXA_WATER, WXA_FIRE, WXA_GRASS, WXA_ICE, WXA_ELECTRIC, WXA_ROCK };

#define WXM_HOT_C10  294   // 85 F: a hot day
#define WXM_TOO_HOT_C10 322 // 90 F: too hot for ice types

static inline uint8_t wxAffinity(int16_t dex) {
  return (dex >= 1 && dex <= 151) ? WX_AFFINITY[dex] : WXA_NONE;
}

static inline bool wxWet(uint8_t wx) { return wx == WX_RAIN || wx == WX_STORM; }

// mood toward the weather, and the line to show (nullptr = nothing to say)
static inline int8_t wxMood(int16_t dex, uint8_t wx, int16_t temp10, const char **msg) {
  const char *m = nullptr;
  int8_t mood = 0;
  // borrowed New York snow is scenery, not the weather here: no mood
  if (wx == WX_NONE || wx == WX_SNOW_NY) { if (msg) *msg = nullptr; return 0; }
  bool snow = wxIsSnow(wx);
  bool hot = temp10 >= WXM_HOT_C10;
  switch (wxAffinity(dex)) {
    case WXA_WATER:
      if (wxWet(wx)) { mood = 1; m = "It loves the rain!"; }
      break;
    case WXA_GRASS:
      if (wxWet(wx)) { mood = 1; m = "Soaking up the rain!"; }
      break;
    case WXA_FIRE:
      if (wxWet(wx)) { mood = -1; m = "Not a fan of rain..."; }
      else if (snow) { mood = -1; m = "Brr... too cold!"; }
      else if (wx == WX_CLEAR && hot) { mood = 1; m = "Loving this heat!"; }
      break;
    case WXA_ICE:
      if (snow) { mood = 1; m = "Snow day!!"; }
      else if (temp10 >= WXM_TOO_HOT_C10) { mood = -1; m = "Too hot... melting..."; }
      break;
    case WXA_ELECTRIC:
      if (wx == WX_STORM) { mood = 1; m = "Charged up by the storm!"; }
      break;
    case WXA_ROCK:
      if (wxWet(wx)) { mood = -1; m = "Doesn't like getting wet"; }
      break;
  }
  if (!m) {                       // everyone else: a calm remark, no effect
    if (wxWet(wx)) m = "Listening to the rain...";
    else if (snow) m = "Watching the snow fall";
    else if (wx == WX_FOG) m = "Lost in the fog...";
  }
  if (msg) *msg = m;
  return mood;
}
