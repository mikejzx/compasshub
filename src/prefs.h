#ifndef COH_PREFS_H
#define COH_PREFS_H

/*
 * A structure representing data stored
 * in preferences file.
 */

struct prefs
{
	// Strings we need to connect.
	std::string hostname;
	std::string path_login;
	std::string path_auth;
	std::string path_tt;
	std::string path_logoff;

	// Aliases for title strings.
	std::unordered_map<std::string, std::string> aliases;
};

#endif
