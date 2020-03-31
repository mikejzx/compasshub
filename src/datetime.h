#ifndef COH_DATETIME
#define COH_DATETIME

/*
 * A wrapper around std::tm.
 */

struct datetime
{
	// Main time_t structure.
	std::time_t time_utc;

	// Construct.
	datetime()
		: time_utc(std::time(0))
	{}
	datetime(const std::time_t& t)
		: time_utc(t)
	{}

	datetime(const datetime& t)
		: time_utc(t.time_utc)
	{}

	datetime(const int d, const int m, const int y,
		const int hr=0, const int min=0)
	{
		std::tm* t;
		std::time_t raw;

		// Fill these in and call mktime.
		time(&raw);
		t = gmtime(&raw);
		t->tm_year = y - 1900;
		t->tm_mon  = m - 1;
		t->tm_mday = d;

		// Optional hour/minutes.
		t->tm_hour = hr;
		t->tm_min  = min;
		t->tm_sec  = 0;

		time_utc = mktime(t);
	}

	// Get the local time.
	inline void get_local_time(int* d, int* m, int* y, int* hr, int* mn) const
	{
		// Convert to local time.
		std::tm t = *(std::localtime(&time_utc));

		// Return info.
		if (d)  { *d  = t.tm_mday; }
		if (m)  { *m  = t.tm_mon + 1; }
		if (y)  { *y  = t.tm_year + 1900; }
		if (hr) { *hr = t.tm_hour; }
		if (mn) { *mn = t.tm_min; }
	}

	// Unsafe local time. This will throw an error if any of the variables are null!
	inline void get_local_time_unsafe(int* d, int* m, int* y, int* hr, int* mn) const
	{
		// Convert to local time.
		std::tm t = *(std::localtime(&time_utc));

		// Return info.
		*d  = t.tm_mday;
		*m  = t.tm_mon + 1;
		*y  = t.tm_year + 1900;
		*hr = t.tm_hour;
		*mn = t.tm_min;
	}

	// Get a time string that looks pretty nice
	// For example:
	// - Today @ 16:00
	// - Yesterday @ 06:00
	// - X days ago
	void get_pretty_string(char* buffer) const
	{
		// Get the local time of this object.
		std::tm t = *(std::localtime(&time_utc));

		// Get current local time.
		std::time_t n = std::time(0);
		std::tm now = *(std::localtime(&n));

		// Check date
		if (
			t.tm_yday == now.tm_yday
			&& t.tm_year == now.tm_year
		)
		{
			// Today, at HH:MM
			sprintf(buffer, "today @ %02u:%02u", t.tm_hour, t.tm_min);
		}
		else if (
			t.tm_yday == (now.tm_yday - 1)
			&& t.tm_year == now.tm_year
		)
		{
			// Yesterday, at
			sprintf(buffer, "yesterday @ %02u:%02u", t.tm_hour, t.tm_min);
		}
		else
		{
			// X days ago
			int diff = now.tm_yday - t.tm_yday + 365 * (now.tm_year - t.tm_year);
			sprintf(buffer, "%u days ago", diff);
		}
	}

	// C++ string alternative of the above. Would be a bit slower.
	inline std::string get_pretty_string(void) const
	{
		char buff[22];
		get_pretty_string(buff);
		return std::string(buff);
	}

	// Comparison operator overloads.

	// Less than
	inline bool operator<(const datetime& other) const
	{
		// If we get a negative difference then left side is smaller.
		return std::difftime(time_utc, other.time_utc) < 0;
	}

	// Less than/equal
	inline bool operator<=(const datetime& other) const
	{
		return std::difftime(time_utc, other.time_utc) <= 0;
	}

	// Greater
	inline bool operator>(const datetime& other) const
	{
		return std::difftime(time_utc, other.time_utc) > 0;
	}

	// Greater/equal
	inline bool operator>=(const datetime& other) const
	{
		return std::difftime(time_utc, other.time_utc) >= 0;
	}

	// Equal
	inline bool operator==(const datetime& other) const
	{
		return std::difftime(time_utc, other.time_utc) == 0;
	}

	// Not equal
	inline bool operator!=(const datetime& other) const
	{
		return std::difftime(time_utc, other.time_utc) != 0;
	}
};

#endif
