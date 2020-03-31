
/*
 * Helper methods.
 */

#include "pch.h"
#include "util.h"

/*
 * This provides a string URL-encoding method which is used to
 * pass URL parameters to the site where needed.
 *
 * Thanks to xperroni for the code:
 * https://stackoverflow.com/a/17708801
 */
std::string util::url_encode(const std::string& value)
{
	using namespace std;

	ostringstream escaped;
	escaped.fill('0');
	escaped << hex;

	for (string::const_iterator i = value.begin(), n = value.end(); i != n; ++i)
	{
		string::value_type c = (*i);

		// Keep alphanumeric and other accepted characters intact
		if (isalnum(c) || c == '-' || c == '_' || c == '.' /* || c == '~'*/)
		{
			escaped << c;
			continue;
		}

		// Any other characters are percent-encoded
		escaped << uppercase;
		escaped << '%' << setw(2) << int((unsigned char) c);
		escaped << nouppercase;
	}

	return escaped.str();
}

// Converts a 3-letter month string to a number.
unsigned util::month_lookup_from_str(const char* month)
{
	// Months in order.
	static const char* MONTHS[12] = {
		"Jan", "Feb", "Mar", "Apr", "May", "Jun",
		"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// Iterate over all the months.
	for (unsigned i = 0; i < 12; ++i)
	{
		if (strcmp(month, MONTHS[i]) == 0)
		{
			return i + 1;
		}
	}
	LOG_WARN("Tried to get a month (%s) that was not in lookup table!", month);
	return 0;
}

// Gets the day of the week as a string.
std::string util::get_day_of_week_str(uint8_t i)
{
	static const char* DAYS_OF_WEEK[] = {
		"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
	};
	return DAYS_OF_WEEK[i];
}
