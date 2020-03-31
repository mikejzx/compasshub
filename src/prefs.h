#ifndef COH_PREFS_H
#define COH_PREFS_H

/*
 * A structure representing data stored
 * in preferences file.
 * Mainly just URLs used to access the site.
 */

struct prefs
{
	std::string hostname;
	std::string path_login;
	std::string path_auth;
	std::string path_tt;
	std::string path_logoff;
};

#endif
