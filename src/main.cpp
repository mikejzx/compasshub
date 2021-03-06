
/*
	- main.cpp
	Entry point of the application.

	Date: Sat, March 14th, 2020
	Author: Michael Skec
	E-mail: skec@protonmail.ch
*/

// Include pre-compiled header and other local includes.
#include "pch.h"

#include "application.h"
#include "net_client.h"
#include "tt_period.h"
#include "wnd_manager.h"

#include "datetime.h"
#include "datetime_dmy.h"
#include "tt_day.h"

static datetime_dmy get_dmy_today(void);
static bool parse_cmd_line(int, char**);

/*
 * Program entry point.
 */
int main (int argc, char* argv[])
{

	// Initialise objects.
	log_initialise();
	// Parse command line arguments.
	if (!parse_cmd_line(argc, argv))
	{
		return 0;
	}

	// Initialise window manager.
	wnd_manager& winman = wnd_manager::get();

	// Create main application state.
	application* app = new application(wnd_manager::cb_date_set);
	app->set_cur_date(get_dmy_today());
	winman.set_app(app);
	if (!app->prefs_check())
	{
		LOG_ERROR("Prefs file either missing or invalid.");

		// Didn't read prefs file. Show the setup prompt.
		winman.show_setup_prompt();
		return 0;
	}

	// Initialise client with URLs.
	net_client* client = new net_client(
		app->get_prefs(), wnd_manager::cb_login_status_changed
	);
	app->client_set(client);

	// Perform initial draw.
	winman.redraw_initial();

	// Window manager loop.
	while (winman.update());

	// Cleanup.
	delete client;
	delete app;

	// Terminated with success.
	LOG_INFO("Terminating...");
	return 0;
}

// Get today's date as datetime_dmy
static datetime_dmy get_dmy_today(void)
{
	// Get current local time.
	std::time_t n = std::time(0);
	std::tm now = *(std::localtime(&n));

	// Return object with the correct offsets.
	return datetime_dmy(
		now.tm_mday,
		now.tm_mon + 1,
		now.tm_year + 1900,
		now.tm_wday
	);
}

// Parse command line arguments.
// Returns false if program execution should end.
static bool parse_cmd_line(int argc, char* argv[])
{
	(void)argc;

	// Iterate until no arguments left.
	while (*++argv)
	{
		// We only look for a version string for now.
		if (strcmp(*argv, "--version") == 0 ||
			strcmp(*argv, "-v") == 0)
		{
			printf(COH_PROGRAM_NAME " - " COH_PROGRAM_VERSION "\n");
			return false;
		}
	}

	return true;
}
