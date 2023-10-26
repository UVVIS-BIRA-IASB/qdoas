#ifndef DATE_UTIL_H
#define DATE_UTIL_H
#include <string>
#include <time.h>

time_t parse_utc_date(const std::string& utc_date);

void get_utc_date(time_t ref_time, int delta_t, struct datetime *date_time);

#endif
