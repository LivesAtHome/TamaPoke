// Holiday add-on: which holiday (if any) the local date falls in. Pure date math,
// no hardware, so it is checked on a desktop (test/test_holiday.cpp).
#pragma once
#include <stdint.h>

enum Holiday : uint8_t {
  HOL_NONE = 0,
  HOL_NEWYEAR,      // Dec 27 - Jan 1   fireworks at night, confetti by day
  HOL_VALENTINE,    // Feb 7 - 14       hearts drifting up
  HOL_STPATRICK,    // Mar 10 - 17      shamrocks in the grass
  HOL_EASTER,       // the week to Easter Sunday: painted eggs on the ground
  HOL_JULY4,        // Jun 27 - Jul 4   bunting, fireworks at night
  HOL_HALLOWEEN,    // Oct 24 - 31      jack-o'-lanterns, bats at night
  HOL_THANKSGIVING, // week before Thu - Sun: autumn leaves falling
  HOL_CHRISTMAS,    // Dec 18 - 26      string lights, a little tree
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

// days since 1970-01-01 for a civil date (inverse of holCivil)
static inline int32_t holDays(int y, int m, int d) {
  y -= m <= 2;
  int32_t era = (y >= 0 ? y : y - 399) / 400;
  uint32_t yoe = (uint32_t)(y - era * 400);
  uint32_t doy = (153 * (m > 2 ? m - 3 : m + 9) + 2) / 5 + d - 1;
  uint32_t doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  return era * 146097 + (int32_t)doe - 719468;
}

#define HOL_LEAD_DAYS 7   // decorations go up a week before the day itself

// which holiday the date is in; *mainDay = true on the holiday itself.
// Each one shows from a week before its day until its last day; the order
// below settles the two overlaps (St. Patrick's before an early Easter, and
// Christmas ends on the 26th so New Year's starts on the 27th).
static inline uint8_t holidayFor(int y, int m, int d, bool *mainDay) {
  struct Win { uint8_t h; int32_t day, from, to; };
  int32_t t = holDays(y, m, d);
  int em, ed;
  holEaster(y, em, ed);
  int32_t easter = holDays(y, em, ed), thanks = holDays(y, 11, holThanksgivingDay(y));
  int32_t xmas = holDays(y, 12, 25), ny = holDays(m == 12 ? y + 1 : y, 1, 1);
  const Win w[] = {
    { HOL_NEWYEAR, ny, holDays(m == 12 ? y : y - 1, 12, 27), ny },
    { HOL_VALENTINE, holDays(y, 2, 14), holDays(y, 2, 14) - HOL_LEAD_DAYS, holDays(y, 2, 14) },
    { HOL_STPATRICK, holDays(y, 3, 17), holDays(y, 3, 17) - HOL_LEAD_DAYS, holDays(y, 3, 17) },
    { HOL_EASTER, easter, easter - HOL_LEAD_DAYS, easter },
    { HOL_JULY4, holDays(y, 7, 4), holDays(y, 7, 4) - HOL_LEAD_DAYS, holDays(y, 7, 4) },
    { HOL_HALLOWEEN, holDays(y, 10, 31), holDays(y, 10, 31) - HOL_LEAD_DAYS, holDays(y, 10, 31) },
    { HOL_THANKSGIVING, thanks, thanks - HOL_LEAD_DAYS, thanks + 3 },
    { HOL_CHRISTMAS, xmas, xmas - HOL_LEAD_DAYS, xmas + 1 },
  };
  for (const Win &x : w)
    if (t >= x.from && t <= x.to) {
      if (mainDay) *mainDay = (t == x.day);
      return x.h;
    }
  if (mainDay) *mainDay = false;
  return HOL_NONE;
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
