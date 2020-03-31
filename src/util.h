#ifndef COH_UTIL_H
#define COH_UTIL_H

/*
 * Random utility methods.
 */

namespace util
{
	// Encode URL
	extern std::string url_encode(const std::string&);

	// Converts a 3-letter month string to a number.
	extern unsigned month_lookup_from_str(const char*);

	// Gets a day of week string.
	extern std::string get_day_of_week_str(uint8_t);
}

#endif
