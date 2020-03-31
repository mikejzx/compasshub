
#include "pch.h"
#include "cookie.h"
#include "datetime.h"

// Construct cookie from string.
cookie::cookie(const std::string& s)
{
	LOG_DBUG("Constructing cookie: %s", s.c_str());

	expiry_init = false;

	// Get the identifier.
	char* buffer = (char*)alloca(s.find(";") + 1);
	if (!buffer)
	{
		LOG_ERROR("Out of memory!");
		return;
	}
	sscanf(s.c_str(), "%s; %*s", buffer);

	// Move into our buffer.
	id = std::string(buffer);

	// Try to get the expiry date if it exists.
	size_t p = s.find("expires=");
	if (p != std::string::npos)
	{
		// Parse the expiry date.
		char emon_str[4];
		unsigned eday, eyr;
		sscanf(s.c_str() + p, "expires=%*03s, %02u-%03s-%u %*s",
			&eday, emon_str, &eyr);

		// If the year was 2 digit, add the thousands.
		if (eyr < 2000)
		{
			eyr += 2000;
		}

		// Convert month to a number.
		unsigned emon = util::month_lookup_from_str(emon_str);

		// Set expiry date.
		expiry = datetime(eday, emon, eyr);
		expiry_init = true;
	}
}
