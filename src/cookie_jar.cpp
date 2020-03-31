
/*
 * cookie_jar.cpp
 * - Stores cookies and provides helper methods
 *   associated with them.
 */

#include "pch.h"
#include "cookie.h"
#include "cookie_jar.h"

// Constructor.
cookie_jar::cookie_jar()
{
	// Initialise to null.
	expiry_earliest_init = false;
	loaded_from_disk = false;

	// Check if we already have a cookie jar on disk.
	load_cookies_from_disk();

	// If we didn't loaded any cookies, reserve for some objects.
	if (!loaded_from_disk)
	{
		// Reserve for 7 cookies.
		content.reserve(5);
	}
}

// Called upon deletion.
cookie_jar::~cookie_jar()
{
	// Free all the memory used by cookies.
	LOG_INFO("Freeing cookie jar...");
	for (unsigned i = 0; i < content.size(); ++i)
	{
		delete content[i];
	}
}

// Add a cookie to the cookie jar.
void cookie_jar::add_cookie(cookie* const c)
{
	// Ignore if we loaded from disk.
	if (loaded_from_disk)
	{
		LOG_WARN("Tried to add a cookie when we are loaded from disk.");
		delete c;
		return;
	}

	content.emplace_back(c);

	// Check if this cookie has an earlier expiry
	// date than the current.
	if (c->expiry_has() && (c->expiry_get() < expiry_earliest || !expiry_earliest_init))
	{
		// Copy the earlier time.
		expiry_earliest_init = true;
		expiry_earliest = datetime(c->expiry_get());
	}
}

// Get concatenated cookie strings.
std::string cookie_jar::get_compound_string(void) const
{
	// If we are loaded from disk, just use the override string.
	if (loaded_from_disk)
	{
		return loaded_disk_override;
	}

	// Not from disk.

	// Just iterate over all of them and add to the string.
	std::string result;
	result.reserve(128);
	for (unsigned i = 0; i < content.size(); ++i)
	{
		result += content[i]->get_id() + " ";
	}
	return result;
}

// Validate all the cookies we have, and save into a file.
void cookie_jar::validate(const char* uid)
{
	if (loaded_from_disk)
	{
		LOG_INFO("Not writing to disk, because we read from it.");
		return;
	}

	LOG_INFO("Writing validated cookies to file.");

	user_id = uid;

	// Just create a file in the same directory.
	// The first line will have the earliest expiry date of all the cookies.
	// If the expiry date has passed, then we discard it and prompt user to sign in
	// again.
	std::ofstream file(COH_COOKIE_JAR_PATH);

	std::tm* e = gmtime(&expiry_earliest.time_utc);
	file << "expiry=" << (e->tm_mday) << "."
		<< (e->tm_mon + 1) << "."
		<< e->tm_year + 1900 << std::endl;
	file << "uid=" << uid << std::endl;
	file << get_compound_string();

	file.close();
}

// Try to load a cookie jar from disk.
void cookie_jar::load_cookies_from_disk()
{
	// Check if file exists.
	std::ifstream f(COH_COOKIE_JAR_PATH);
	if (!f.good())
	{
		LOG_INFO("No cookie jar on disk. Logging in manually.");
		return;
	}

	// Read the cookie jar content.
	std::string ln_expiry;
	std::string ln_userid;
	std::string ln_cookies;
	if (!std::getline(f, ln_expiry)
		|| !std::getline(f, ln_userid)
		|| !std::getline(f, ln_cookies))
	{
		LOG_WARN("Couldn't read all data from disk cookie jar. Prompting for credentials again.");
		return;
	}

	// Parse expiration date.
	unsigned ed, em, ey;
	if (sscanf(ln_expiry.c_str(), "expiry=%u.%u.%u", &ed, &em, &ey) != 3)
	{
		LOG_WARN("Failed to parse cookie jar expiration date on disk. Aborting load...");
		return;
	}

	// Check if the expiry date has passed.
	if (datetime(std::time(0)) >= datetime(ed, em, ey))
	{
		LOG_INFO("Cookie jar is expired. Will not use.");
		return;
	}

	// Parse user id.
	char uid[16];
	if (sscanf(ln_userid.c_str(), "uid=%s", uid) != 1)
	{
		LOG_WARN("Failed to parse cookie jar user ID on disk. Aborting load...");
		return;
	}

	// Make sure we have cookies.
	if (ln_cookies.length() < 1)
	{
		LOG_WARN("Cookie jar on disk missing cookies. Aborting load...");
		return;
	}

	// Store all the values.
	loaded_from_disk = true;
	user_id = uid;
	loaded_disk_override = ln_cookies;

	//LOG_INFO("We are loaded from disk! UID: %s, cookie override: %s", user_id.c_str(), loaded_disk_override.c_str());
}
