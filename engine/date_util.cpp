#include <sstream>
#include <string>

#include <time.h>

#ifdef _MCS_VER
#define timegm _mkgmtime
#endif

// Read year, month and day from an iso formatted date string utc_date, and convert this to a unix timestamp.
time_t parse_utc_date(const std::string& utc_date) {
  int year,month,day;
  std::istringstream utc(utc_date);

  utc >> year;
  utc.ignore(1,'-');
  utc >> month;
  utc.ignore(1,'-');
  utc >> day;

  struct tm t = {
    0,  // seconds of minutes from 0 to 61
    0,  // minutes of hour from 0 to 59
    0,  // hours of day from 0 to 24
    day,  // day of month from 1 to 31
    month - 1,  // month of year from 0 to 11
    year - 1900, // year since 1900
    0,  // days since sunday
    0,  // days since January 1st
    0, // have daylight savings time?
#if defined(__GNUC__) && !defined(__MINGW32__) // initialize extra fields available in GCC but not in MinGW32
    0, // Seconds east of UTC
    0  // Timezone abbreviation
#endif
  };

  return timegm(&t);
}
