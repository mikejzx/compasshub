
#ifndef COH_COOKIE_JAR_H
#define COH_COOKIE_JAR_H

/*
 * cookie_jar.h
 * - Stores cookies and provides helper methods
 *   associated with them.
 */

#include "datetime.h"

struct cookie;

class cookie_jar
{
public:
	cookie_jar();
	~cookie_jar();

	// Add a cookie to the cookie jar.
	void add_cookie(cookie* const);

	// Get a string of all the cookies concatenated together.
	std::string get_compound_string(void) const;

	// Get user ID
	inline std::string get_user_id(void) const 
	{
		return user_id;
	}

	// Get loaded from disk status.
	inline bool is_loaded_from_disk(void) const
	{
		return loaded_from_disk;
	}

	// Mark all the cookies as validated, meaning
	// everything is there, and can be re-used later on.
	// This method will write the cookies to a file which will
	// be loaded with the program.
	void validate(const char*);

private:
	// List of cookies.
	std::vector<cookie*> content;

	// User ID string.
	std::string user_id;
	
	// The earliest expiry date in cookie jar.
	// If this expires, we assume the whole thing
	// is expired.
	datetime expiry_earliest; 
	// Whether the above has been initialised or not.
	bool expiry_earliest_init;

	// Whether the settings were loaded from the disk.
	// We need this flag as the load is very partial,
	// as cookies themselves are not objects :w
	bool loaded_from_disk;
	std::string loaded_disk_override;

	// Attempt to load a cookie jar from disk.
	// If we can and they aren't expired, they will be used.
	void load_cookies_from_disk(void);
};

#endif
