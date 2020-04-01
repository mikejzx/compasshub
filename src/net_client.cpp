/*
 * net_client.cpp
 * This class provides a nice interface to access
 * Compass data.
 */

#include "pch.h"
#include "cookie.h"
#include "cookie_jar.h"
#include "datetime_dmy.h"
#include "net_client.h"
#include "prefs.h"
#include "tt_parser.h"
#include "tt_period.h"

// Typedef this long type.
typedef std::shared_ptr<httplib::Response> http_resp;

// Use this macro in request methods.
// Need to make sure the response variable is called "resp".
// (Undefined at bottom.)
#define S_CHK_RESP(mode)\
	/* Check if resp is null */\
	if (!resp)\
	{ LOG_ERROR("Error! Did not " mode); return false; }\
	/* Make sure we got a HTTP 200 (or a 302 redirect) */\
	LOG_INFO("Response: HTTP %d", resp->status);\
	if (resp->status != 200 && resp->status != 302)\
	{ LOG_ERROR(mode " request failed."); return false; }\


// Construct new client.
net_client::net_client(const prefs& p,
		void(*cb_lchg)(int))
	: sslclient(nullptr), cookies(nullptr), hostname(p.hostname),
	path_login(p.path_login), path_auth(p.path_auth),
	path_timetable(p.path_tt), path_logoff(p.path_logoff)
{
	// Print out the URLs to log file.
	LOG_INFO("Initialising net_client with: \n"
		"+ hostname:       %s\n"
		"+ path_login:     %s\n"
		"+ path_auth:      %s\n"
		"+ path_timetable: %s\n"
		"+ path_logoff:    %s\n",
		p.hostname.c_str(), p.path_login.c_str(), p.path_auth.c_str(),
		p.path_tt.c_str(), p.path_logoff.c_str()
	);

	// Headers
	std::string header_origin = "https://" + hostname;
	httplib::Headers headers_base = {
		{ "Accept-Language", "en-GB,en;q=0.5" },
		{ "Connection",      "keep-alive" },
		{ "DNT",             "1" },
		{ "Host",            hostname },
		{ "Origin",          header_origin },
		{ "Referer",         header_origin + path_login },
		{ "User-Agent",      "Mozilla/5.0 (Linux; rv:70) Firefox/73" }
	};

	// Initialise cookie jar.
	cookies = new cookie_jar();

	// Set our login callback.
	on_chg_login = cb_lchg;

	// If we loaded from disk, set the status.
	if (cookies->is_loaded_from_disk())
	{
		on_chg_login(COH_STATUS_READDISK);
	}
	else
	{
		// Not logged in.
		on_chg_login(COH_STATUS_LOGGEDOFF);
	}
}

// Clean up.
net_client::~net_client()
{
	LOG_INFO("Cleaning up net_client...");

	// Free SSLClient memory if existant.
	if (sslclient_exists())
	{
		delete sslclient;
		LOG_INFO("Freed SSLClient.");
	}

	// Free cookie jar.
	if (cookies)
	{
		delete cookies;
	}
}

// Log into Compass site.
bool net_client::login(
	const std::string& username,
	const std::string& password
)
{
	// If we are loaded from disk, we don't need to log in again.
	if (cookies->is_loaded_from_disk())
	{
		LOG_INFO("Already loaded from disk. Aborting login.");
		return true;
	}

	LOG_INFO("Logging in with username '%s'...", username.c_str());

	// Create SSLClient if we need.
	if (!sslclient_check())
	{
		LOG_ERROR("Cannot login. SSLClient failed to create.");
		return false;
	}

	// Steps to log in:
	// 0.) GET request to login url for cookies.
	// 1.) POST request to auth url, for another cookie we need.
	// 2.) POST to the login URL with credentials.

	// Macros we will need.
	// (May move S_ADD_COOKIES to be a proper method in cookie_jar class)
#define S_ADD_COOKIES()\
	/* Iterate over all the cookies and add to cookie jar. */\
	for (size_t i = 0; i < resp->get_header_value_count("set-cookie"); ++i)\
	{ cookies->add_cookie(new cookie(resp->get_header_value("set-cookie", i))); }

	// Send the first GET, accepting text/html
	http_resp resp;
	{
		httplib::Headers headers_get_login = headers_base;
		headers_get_login.emplace("Accept", "text/html");

		// Send a GET request to login url.
		resp = sslclient->Get(path_login.c_str(), headers_get_login);
	}

	// Make sure we got a 200, and add cookies.
	S_CHK_RESP("GET");
	S_ADD_COOKIES();

	// Send the POST to get authenticated.
	{
		httplib::Headers headers_post_auth = headers_base;
		headers_post_auth.emplace("Accept", "*/*");
		headers_post_auth.emplace("Cookie", cookies->get_compound_string());

		// Create the JSON payload we want to post.
		std::string post_payload_auth = "{\"username\":\"" + username + "\"}";
		LOG_DBUG("POST data: %s", post_payload_auth.c_str());

		// Send a POST request to auth url.
		resp = sslclient->Post(path_auth.c_str(), headers_post_auth, post_payload_auth, "application/json");
	}

	// Check if POST succeeded.
	S_CHK_RESP("POST");
	S_ADD_COOKIES();

	// Check if a Captcha is required.
	if (resp->body.compare("{\"d\":false}") == 0)
	{
		LOG_DBUG("No authentication required. Can proceed with login.");
	}
	else if (resp->body.compare("{\"d\":true}") == 0)
	{
		LOG_WARN("Captcha required... Cannot proceed.");
		return false;
	}
	else
	{
		LOG_WARN("Unrecognised Captcha response, '%s'", resp->body.c_str());
		return false;
	}

	// Add username cookie.
	cookies->add_cookie(new cookie("username=" + username + ";"));

	// Log into the damn site!
	{
		httplib::Headers headers_post_login = headers_base;
		headers_post_login.emplace("Accept", "text/html");
		headers_post_login.emplace("Cookie", cookies->get_compound_string());

		// Create URL-encoded payload.
		std::string encod_user = util::url_encode(username);
		std::string encod_pass = util::url_encode(password);
		std::string post_payload_login = "__EVENTTARGET=button1&username=" + encod_user + "&password=" + encod_pass;
		LOG_DBUG("POST data: %s", post_payload_login.c_str());

		// Send a POST request to login url.
		resp = sslclient->Post(path_login.c_str(), headers_post_login, post_payload_login, "application/x-www-form-urlencoded");
	}

	// Check if POST succeeded.
	S_CHK_RESP("POST");

	sslclient->set_follow_location(true);

	// Get the home page to A.) Check if we were logged in properly,
	// and B.) get our User ID for schedule requests.
	{
		httplib::Headers headers_get_home = headers_base;
		headers_get_home.emplace("Accept", "text/html");
		headers_get_home.emplace("Cookie", cookies->get_compound_string());

		// Send a GET request to home url.
		resp = sslclient->Get("/", headers_get_home);
	}

	// Make sure we got a 200, and add cookies.
	S_CHK_RESP("GET");

	// Try to get the User ID from the page.
	// It is embedded in the page's JavaScript so we just search for it.
	const std::string USER_ID_SEARCH = "Compass.organisationUserId = ";
	const size_t USER_ID_SEARCH_LEN = USER_ID_SEARCH.length();
	size_t useridindex = resp->body.find(USER_ID_SEARCH);
	if (useridindex == std::string::npos)
	{
		LOG_ERROR("Couldn't get the user ID from the home page. Cannot retrieve schedule.");
		return false;
	}
	useridindex += USER_ID_SEARCH_LEN;
	std::string userid = resp->body.substr(useridindex, 32);

	// Find the semicolon starting from last index.
	size_t useridindex_smcln = userid.find(";");
	if (useridindex_smcln == std::string::npos)
	{
		LOG_ERROR("Failed to parse User ID.");
		return false;
	}
	userid[useridindex_smcln] = '\0';
	LOG_INFO("Got user id: '%s'", userid.c_str());

	// We have all the cookies we need. Mark them as validated
	// so they can be re-used until expiry date..
	// (Will be cancelled if we are loaded from disk already.)
	cookies->validate(userid.c_str());

	// Undefine macros
#undef S_ADD_COOKIES

	// We are logged in.
	chg_login_status(COH_STATUS_LOGGEDIN);

	// Successful login.
	return true;
}

// Get the timetable information for date.
bool net_client::retrieve_data(
	std::vector<tt_period>& timetable,
	std::vector<tt_period>& events,
	const datetime_dmy& dt,
	const prefs& pref
)
{
	// Create SSLClient if we need.
	if (!sslclient_check())
	{
		LOG_ERROR("Cannot retrieve data. SSLClient failed to create.");
		return false;
	}

	// We need to be sure that we're able to actually request
	// or not. If we just logged in we can. If we have cookies,
	// we can assume we can.
	if (!logged_in && !cookies->is_loaded_from_disk())
	{
		LOG_ERROR("Cannot retrieve. Neither logged in nor loaded from disk.");
		return false;
	}

	char datestr[16];
	sprintf(datestr, "%04d-%02d-%02d", dt.year, dt.month, dt.day);
	LOG_INFO("Attempting to retrieve data for date, %s", datestr);

	// Send the POST to get information.
	http_resp resp;
	{
		httplib::Headers headers_post_json = headers_base;
		headers_post_json.emplace("Accept", "*/*");
		headers_post_json.emplace("Cookie", cookies->get_compound_string());

		// Create the JSON payload we want to post.
		std::string post_payload_json = "{\"startDate\":\"";
		post_payload_json += datestr;
		post_payload_json += "\",\"endDate\":\"";
		post_payload_json += datestr;
		post_payload_json += "\",\"page\":1,\"userId\":";
		post_payload_json += cookies->get_user_id();
		post_payload_json += "}";

		LOG_DBUG("POST data: %s", post_payload_json.c_str());

		// Send a POST request to the timetable url.
		resp = sslclient->Post(path_timetable.c_str(), headers_post_json, post_payload_json, "application/json");
	}

	// Check if POST succeeded.
	S_CHK_RESP("POST");

	// Parse the JSON.
	if (!tt_parser::parse_json(timetable, events, resp->body, pref))
	{
		LOG_ERROR("Unable to retrieve timetable due to JSON errors.");
		timetable.clear();
		events.clear();
		return false;
	}
	LOG_INFO("Timetable data retrieved. Got (%u) periods.", timetable.size());

	// We can say that we are logged in.
	// We just set the state in case we were loaded
	// from disk.
	chg_login_status(COH_STATUS_LOGGEDIN);

	return true;
}

// Create the SSLClient.
bool net_client::sslclient_create(void)
{
	// Create the SSLClient.
	sslclient = new httplib::SSLClient(hostname, COH_PORT_HTTPS);
	sslclient->set_ca_cert_path(COH_CA_CERT_PATH);
	sslclient->enable_server_certificate_verification(true);

	// Check for errors. (Wrong spot?)
	auto result = sslclient->get_openssl_verify_result();
	if (result)
	{
		LOG_ERROR("OpenSSL verify error: %s", X509_verify_cert_error_string(result));
		return false;
	}

	return !!sslclient;
}

// Change the login status.
void net_client::chg_login_status(int to)
{
	static int login_status = -1;

	// logged_in means we are 100% logged into the site.
	logged_in = (to == COH_STATUS_LOGGEDIN);

	// Call the callback if status actually changed.
	if (to != login_status)
	{
		on_chg_login(to);
		login_status = to;
	}
}

// Undefine this macro.
#undef S_CHK_RESP
