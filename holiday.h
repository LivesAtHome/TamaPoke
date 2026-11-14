// Holiday add-on: which holiday (if any) the local date falls in. Pure date math,
// no hardware, so it is checked on a desktop (test/test_holiday.cpp).
#pragma once
#include <stdint.h>

enum Holiday : uint8_t {
  HOL_NONE = 0,
  HOL_NEWYEAR,      // Dec 31 - Jan 1   fireworks at night, confetti by day
  HOL_VALENTINE,    // Feb 13 - 14      hearts drifting up
  HOL_STPATRICK,    // Mar 17           shamrocks in the grass
  HOL_EASTER,       // Sat - Sun        painted eggs on the ground
  HOL_JULY4,        // Jul 3 - 4        bunting, fireworks at night
  HOL_HALLOWEEN,    // Oct 25 - 31      jack-o'-lanterns, bats at night
  HOL_THANKSGIVING, // Thu - Sun        autumn leaves falling
  HOL_CHRISTMAS,    // Dec 20 - 26      string lights, a little tree
  HOL_COUNT
};

// civil date from days since 1970-01-01 (Howard Hinnant's algorithm)
static inline void holCivil(int32_t z, int &y, int &m, int &d) {
  z += 719468;
  int32_t era = (z >= 0 ? z : z - 146096) / 146097;
  uint32_t doe = (uint32_t)(z - era * 146097);
  uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  int32_t yy = (int32_t)yoe + era * 400;
  uint32_t doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
  uint32_t mp = (5 * doy + 2) / 153;
  d = (int)(doy - (153 * mp + 2) / 5 + 1);
  m = (int)(mp < 10 ? mp + 3 : mp - 9);
  y = (int)(yy + (m <= 2));
}

// day of week, 0 = Sunday (Sakamoto)
static inline int holWeekday(int y, int m, int d) {
  static const int t[] = { 0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4 };
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Western Easter Sunday (anonymous Gregorian algorithm)
static inline void holEaster(int y, int &m, int &d) {
  int a = y % 19, b = y / 100, c = y % 100, dd = b / 4, e = b % 4;
  int f = (b + 8) / 25, g = (b - f + 1) / 3, h = (19 * a + b - dd - g + 15) % 30;
  int i = c / 4, k = c % 4, l = (32 + 2 * e + 2 * i - h - k) % 7;
  int mm = (a + 11 * h + 22 * l) / 451;
  m = (h + l - 7 * mm + 114) / 31;
  d = ((h + l - 7 * mm + 114) % 31) + 1;
}

// US Thanksgiving: the fourth Thursday of November
static inline int holThanksgivingDay(int y) {
  int first = holWeekday(y, 11, 1);              // weekday of Nov 1
  int firstThu = 1 + (4 - first + 7) % 7;
  return firstThu + 21;
}

// which holiday the date is in; *mainDay = true on the holiday itself
static inline uint8_t holidayFor(int y, int m, int d, bool *mainDay) {
  bool md = false;
  uint8_t h = HOL_NONE;
  if ((m == 12 && d == 31) || (m == 1 && d == 1)) { h = HOL_NEWYEAR; md = (m == 1); }
  else if (m == 2 && (d == 13 || d == 14)) { h = HOL_VALENTINE; md = d == 14; }
  else if (m == 3 && d == 17) { h = HOL_STPATRICK; md = true; }
  else if (m == 7 && (d == 3 || d == 4)) { h = HOL_JULY4; md = d == 4; }
  else if (m == 10 && d >= 25) { h = HOL_HALLOWEEN; md = d == 31; }
  else if (m == 11) {
    int t = holThanksgivingDay(y);
    if (d >= t && d <= t + 3) { h = HOL_THANKSGIVING; md = d == t; }
  } else if (m == 12 && d >= 20 && d <= 26) { h = HOL_CHRISTMAS; md = d == 25; }
  if (h == HOL_NONE && (m == 3 || m == 4)) {      // Easter moves: Saturday + Sunday
    int em, ed;
    holEaster(y, em, ed);
    int sm = em, sd = ed - 1;                      // the Saturday before
    if (sd == 0) { sm = 3; sd = 31; }
    if ((m == em && d == ed) || (m == sm && d == sd)) { h = HOL_EASTER; md = (d == ed && m == em); }
  }
  if (mainDay) *mainDay = md;
  return h;
}

// from a LOCAL epoch (the RTC keeps local time, as the rest of the game assumes)
static inline uint8_t holidayForEpoch(uint32_t localEpoch, bool *mainDay) {
  if (!localEpoch) { if (mainDay) *mainDay = false; return HOL_NONE; }
  int y, m, d;
  holCivil((int32_t)(localEpoch / 86400), y, m, d);
  return holidayFor(y, m, d, mainDay);
}

static inline const char *holidayGreeting(uint8_t h) {
  switch (h) {
    case HOL_NEWYEAR: return "Happy New Year!";
    case HOL_VALENTINE: return "Happy Valentine's!";
    case HOL_STPATRICK: return "Happy St. Patrick's!";
    case HOL_EASTER: return "Happy Easter!";
    case HOL_JULY4: return "Happy 4th of July!";
    case HOL_HALLOWEEN: return "Happy Halloween!";
    case HOL_THANKSGIVING: return "Happy Thanksgiving!";
    case HOL_CHRISTMAS: return "Merry Christmas!";
    default: return nullptr;
  }
}
