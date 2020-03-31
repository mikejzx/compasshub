#ifndef COH_NET_CLIENT_H
#define COH_NET_CLIENT_H

/*
 * This class is used to interface with the
 * Compass site.
 */

// Certificate path. Just use the bundled one.
#define COH_CA_CERT_PATH "./ca-bundle.crt"
#define COH_PORT_HTTPS 443

// Forward declare these.
class cookie_jar;
struct cookie;
struct datetime_dmy;
struct prefs;
struct tt_period;

class net_client
{
public:
	// Initialise the client.
	net_client(const prefs&,
		void(*cb_lchg)(int)=[](int b){}
	);
	~net_client();

	// Log into the site
	bool login(const std::string&, const std::string&);

	// Get timetable information.
	bool retrieve_data(std::vector<tt_period>&, std::vector<tt_period>&, const datetime_dmy&);

	// Log out of site.
	void logoff(void);

private:
	httplib::SSLClient* sslclient; // Our main HTTPS client.
	cookie_jar* cookies;           // Store cookies here.
	std::string hostname;          // Host domain.
	std::string path_login;        // Login path.
	std::string path_auth;         // Authentication path.
	std::string path_timetable;    // Timetable request path.
	std::string path_logoff;       // Logoff path.

	// Header stuff used for all requests.
	std::string header_origin;
	httplib::Headers headers_base;

	// Whether we are logged in or not.
	bool logged_in;

	// Callbacks.
	void(*on_chg_login)(int);

private:
	// Creates the SSL client.
	bool sslclient_create(void);

	// Login status changed.
	void chg_login_status(int);

	// Return whether the SSLClient exists.
	inline bool sslclient_exists(void) const
	{
		return !!sslclient;
	}

	// Check for SSLClient exist. If not, try create.
	inline bool sslclient_check(void)
	{
		if (!sslclient_exists())
		{
			return sslclient_create();
		}
		return true;
	}
};

#endif
