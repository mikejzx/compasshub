
#ifndef COH_COOKIE_H
#define COH_COOKIE_H

/*
 * cookie.h
 * - Represents a single cookie object.
 */

#include "datetime.h"

struct cookie
{
	// Construct cookie from string.
	cookie(const std::string& s);

	// Get the identifier
	inline std::string get_id() const
	{
		return id;
	}

	// Does this cookie expire?
	inline bool expiry_has(void) const
	{
		return expiry_init;
	}

	// Get the expiry date.
	inline datetime expiry_get(void) const
	{
		return expiry;
	}

private:
	std::string id;
	datetime expiry;
	bool expiry_init;
};

#endif
