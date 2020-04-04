
/*
 * wnd_manager.cpp
 * The window manager.
 */

#include "pch.h"
#include "anchor.h"
#include "application.h"
#include "datetime.h"
#include "datetime_dmy.h"
#include "net_client.h"
#include "tt_day.h"
#include "tt_period.h"
#include "vec2.h"
#include "window.h"
#include "window_main.h"
#include "window_second.h"
#include "wnd_manager.h"

// Constructor.
wnd_manager::wnd_manager()
{
	// Initialise ncurses TUI.
	ncurses_init();

	// Refresh standard screen.
	wclear(stdscr);
	wrefresh(stdscr);

	// Create our windows.

	// Header
	window* whead = new window({ 1, COH_WND_STRETCH }, { 0, 0 }, anchor::TOP);
	whead->colour_bg_set(COH_COL_HEADER);
	whead->add_str(COH_WND_HEADER_TEXT, { 0, 0 }, ANCH_TL);
	whead->add_str(COH_PROGRAM_CREDIT, { 0, 0 }, ANCH_TR);

	// Main
	window_main* wmain = new window_main({ COH_WND_STRETCH, 100 },  { 2, 0 }, anchor::LEFT, { 1, 0, 2, 0 });
	wmain_str_load     = wmain->add_str("", { 4, 0 }, ANCH_CTR_T);

	// Events
	window* wevnt    = new window_second({ COH_WND_STRETCH, 70 }, { 0, 2 }, anchor::RIGHT, { 1, 0, 2, 0 });
	wevnt->add_str("Events:", { 2, 0 }, ANCH_CTR_T);
	wevnt_str_status = wevnt->add_str(COH_SZ_NOEVENTS, { 4, 0 }, ANCH_CTR_T);

	// Footer
	window* wfoot    = new window({ 1, COH_WND_STRETCH }, { 1, 0 }, anchor::BOTTOM);
	wfoot->colour_bg_set(COH_COL_FOOTER);
	wfoot_str_status = wfoot->add_str(COH_SZ_STATUS_LOGGEDOFF, { 0, 2 }, ANCH_BR);
	wfoot_str_retrv  = wfoot->add_str(COH_SZ_RETR_PROMPT, { 0, 2 }, ANCH_BL, A_BOLD);

	// Status line. Most of this is handled manually.
	window* wstat    = new window({ 1, COH_WND_STRETCH }, { 0, 0 }, anchor::BOTTOM);
	wstat_str_status = wstat->add_str("", { 0, 0 }, anchor::LEFT);

	// Add to the windows vector.
	wnds.resize(5);
	wnds[COH_WND_IDX_FOOTER] = wfoot;
	wnds[COH_WND_IDX_MAIN]   = wmain;
	wnds[COH_WND_IDX_HEADER] = whead;
	wnds[COH_WND_IDX_EVENTS] = wevnt;
	wnds[COH_WND_IDX_STATUS] = wstat;

	// Handle window resizes.
	struct sigaction sigact_resize = {
		[](int param)
		{
			(void)param;

			// Call these to reinitialise terminal.
			endwin();
			wrefresh(stdscr);
			wclear(stdscr);
			wrefresh(stdscr);

			// Redraw everything if we aren't too small.
			if (wnd_manager::can_draw())
			{
				wnd_manager::get().on_resize();
			}
		}
	};
	sigaction(SIGWINCH, &sigact_resize, NULL);
}

// Destructor.
wnd_manager::~wnd_manager()
{
	// De-initialise ncurses TUI.
	ncurses_deinit();

	// Delete windows.
	for (unsigned i = 0; i < wnds.size(); ++i)
	{
		delete wnds[i];
	}
}

// Called when we have everything already initialised.
void wnd_manager::redraw_initial(void)
{
	// Refresh all our stuff from cache.
	refresh_from_cache();

	// Make sure we have reasonable size.
    if (wnd_manager::can_draw())
    {
        return;
    }

	// Mark everything as dirty.
	for (unsigned i = 0; i < wnds.size(); ++i)
	{
		wnds[i]->invalidate();
	}

    // Redraw.
    redraw();
}

// Inform all windows that there was a resize.
// We also do a redraw.
void wnd_manager::on_resize(void)
{
	use_default_colors();

	// Redraw all windows.
	for (unsigned i = 0; i < wnds.size(); ++i)
	{
		wnds[i]->on_resize();
		wnds[i]->redraw();
	}
}

// Force draw all windows.
void wnd_manager::redraw(void)
{
	use_default_colors();

	// Redraw invalidated windows.
	for (unsigned i = 0; i < wnds.size(); ++i)
	{
		wnds[i]->redraw();
	}
}

// Update loop, called constantly.
// Return false when the program terminates.
bool wnd_manager::update(void)
{
	// Handle inputs.
	int ch;
	switch(ch = wgetch(get_wnd_main()->get_wndptr()))
	{
		// 'q' to quit.
		case ('q'):
		case ('Q'):
		{
			// Exit program.
			return false;
		}

		// 'r' to refresh.
		case ('r'):
		case ('R'):
		{
			refresh_from_server();
		} break;

		// 'h' to navigate left.
		case ('h'):
		case ('H'):
		{
			// Tell the app to decrement date
			app->cur_date_decr();

			// Refresh from cache.
			refresh_from_cache();
		} break;

		// 'l' to navigate right.
		case ('l'):
		case ('L'):
		{
			// Tell the app to increment date
			app->cur_date_incr();

			// Refresh from cache.
			refresh_from_cache();
		} break;
	}

	return true;
}

// Initialises ncurses.
void wnd_manager::ncurses_init(void)
{
	initscr(); // Initialise screen.
	cbreak();  // Input one char at a time.
	noecho();  // Don't echo our inputs.
	setlocale(LC_ALL, "");

	// Hide the cursor.
	curs_set(COH_WND_CURSOR_HIDDEN);

	// Initialise our colour pairs.
	start_color();
	//         Colour ID               Foregr.       Backgr.
	init_pair(COH_COL_HEADER,          COLOR_BLACK,  COLOR_YELLOW);
	init_pair(COH_COL_MAIN,            COLOR_WHITE,  COLOR_BLACK);

	init_pair(COH_COL_FOOTER,          COLOR_WHITE,  COH_COL_BG_STATUS);
	init_pair(COH_COL_STATUS_LI,       COLOR_GREEN,  COH_COL_BG_STATUS);
	init_pair(COH_COL_STATUS_LD,       COLOR_YELLOW, COH_COL_BG_STATUS);
	init_pair(COH_COL_STATUS_LO,       COLOR_RED,    COH_COL_BG_STATUS);
	init_pair(COH_COL_STATUS_LO,       COLOR_RED,    COH_COL_BG_STATUS);

	init_pair(COH_COL_PERIOD_NORMAL,   COLOR_WHITE,  COLOR_BLUE);
	init_pair(COH_COL_PERIOD_NORMAL_B, COLOR_BLACK,  COLOR_BLUE);
	init_pair(COH_COL_PERIOD_RCHANG,   COLOR_WHITE,  COLOR_RED);
	init_pair(COH_COL_PERIOD_RCHANG_B, COLOR_BLACK,  COLOR_RED);
	init_pair(COH_COL_PERIOD_SUBST,    COLOR_WHITE,  COLOR_RED);
	init_pair(COH_COL_PERIOD_SUBST_B,  COLOR_BLACK,  COLOR_RED);
	init_pair(COH_COL_PERIOD_CANCEL,   COLOR_WHITE,  COLOR_BLACK);
	init_pair(COH_COL_PERIOD_CANCEL_B, COLOR_BLACK,  COLOR_BLACK);

	LOG_INFO("ncurses initialised.");
}

// De-initialises ncurses.
void wnd_manager::ncurses_deinit(void)
{
	// Show cursor.
	curs_set(COH_WND_CURSOR_VISIBLE);

	// Deinitialise screen.
	endwin();

	LOG_INFO("ncurses de-initialised.");
}

// Show the setup prompt
void wnd_manager::show_setup_prompt(void)
{
	// Create the window.
	window swnd = window({ COH_WND_STRETCH, COH_WND_STRETCH }, { 0, 0 });
	swnd.add_str("Not set up yet. Please create the compasshub.prefs file.", { 0, 0 }, ANCH_CTR);
	swnd.add_str("Press any key to exit...", { 1, 0 }, ANCH_CTR);
	swnd.invalidate();

	// Wait for a keypress.
	wgetch(swnd.get_wndptr());
}

// Called whenever the login status changed.
void wnd_manager::cb_login_status_changed(int status)
{
	LOG_INFO("Login status changed to %d", status);

	// Get window text and attribute.
	std::string s;
	int a;
	switch (status)
	{
		// Logged off.
		case (COH_STATUS_LOGGEDOFF):
		{
			s = COH_SZ_STATUS_LOGGEDOFF;
			a = COLOR_PAIR(COH_COL_STATUS_LO);
		} break;

		// Read from disk
		case (COH_STATUS_READDISK):
		{
			s = COH_SZ_STATUS_READDISK;
			a = COLOR_PAIR(COH_COL_STATUS_LD);
		} break;

		// Logged in
		case (COH_STATUS_LOGGEDIN):
		{
			s = COH_SZ_STATUS_LOGGEDIN;
			a = COLOR_PAIR(COH_COL_STATUS_LI);
		} break;
	}

	// Apply to window.
	wnd_manager& wm = wnd_manager::get();
	wm.get_wnd(COH_WND_IDX_FOOTER)->chg_str(wm.get_wfoot_str_status(), s, a);
}

// Called whenever the date is set.
// We don't instantly update it though. Just change title.
// Main timetable view update is done from view_date.
void wnd_manager::cb_date_set(const datetime_dmy& d)
{
	LOG_INFO("Date set to %02d.%02d.%02d", d.day, d.month, d.year);

	// Set the date string of main window.
	wnd_manager& wm = wnd_manager::get();
	wm.get_wnd_main()->set_date(d);
}

// View the date.
void wnd_manager::view_date(const tt_day& t)
{
	// Change status bar retrieve string.
	get_wnd(COH_WND_IDX_FOOTER)->chg_str(get_wfoot_str_retrv(), std::string(COH_SZ_RETR_LAST).append(t.retrieved.get_pretty_string()));

	// Tell the main window to show our date.
	get_wnd_main()->set_date_info(t);
}

// Refresh the date from cache.
void wnd_manager::refresh_from_cache(void)
{
	// Get if we already cached data.
	tt_day o;
	if (app->get_tt_for_day_if_cached(o, app->get_cur_date()))
	{
		view_date(o);
	}
	else
	{
		// Set the text to refresh.
		get_wnd(COH_WND_IDX_FOOTER)->chg_str(get_wfoot_str_retrv(), COH_SZ_RETR_PROMPT);
	}
}

// Refresh from the data on server.
void wnd_manager::refresh_from_server(void)
{
	// Set loading string.
	get_wnd_main()->chg_str(get_wmain_str_load(), COH_SZ_LOADING);

	// Tell the app to get the data.
	// Then we view it here.
	auto l_get_tt = [&]()
	{
		tt_day o;
		if (app->get_tt_for_day_update(o, app->get_cur_date()))
		{
			// Hide the loading string.
			get_wnd_main()->chg_str(get_wmain_str_load(), "");

			// View the timetable.
			view_date(o);

			// Done.
			return true;
		}
		return false;
	};

	// If we get it, just return.
	if (l_get_tt()) { return; }

	// Hide loading string.
	get_wnd_main()->chg_str(get_wmain_str_load(), "");

	// Ask for user's credentials.
	char cred_user[11];
	char cred_pass[65];
	if (!get_credentials(cred_user, cred_pass))
	{
		return;
	}

	// Update string.
	window* const w = get_wnd(COH_WND_IDX_STATUS);
	w->chg_str(wstat_str_status, "Logging in...");

	// Try login with what we got.
	app->client_get()->login(cred_user, cred_pass);

	// Try get schedule.
	std::string strlogin_status;
	int strcolour;
	if (l_get_tt())
	{
		strlogin_status = "Successful login.";
		strcolour = COH_COL_STATUS_LI;
	}
	else
	{
		strlogin_status = "Failed to log in.";
		strcolour = COH_COL_STATUS_LO;
	}
	w->chg_str(wstat_str_status, strlogin_status, COLOR_PAIR(strcolour));

	// Wait for input to hide string.
	wgetch(w->get_wndptr());
	w->chg_str(wstat_str_status, "");
}

// Ask for user's credentials.
bool wnd_manager::get_credentials(char* user, char* pass)
{
	// Get window where data is entered.
	static window& cwnd = *get_wnd(COH_WND_IDX_STATUS);
	static WINDOW* cwptr = cwnd.get_wndptr();
	cwnd.invalidate();

	// Enable cursor and echoing.
	curs_set(COH_WND_CURSOR_VISIBLE);

	// Used for getting string input.
	auto l_get_str = [&](char* buf, bool is_pass, size_t maxlen, size_t startpos)
	{
		// Position of cursor.
		size_t pos = 0;
		while (true)
		{
			bool bksp = false;
			chtype ch;
			switch(ch = wgetch(cwptr))
			{
				// Handle return key.
				case (KEY_ENTER):
				case (0x0A):
				case (0x0D):
				{
					return;
				}

				// Handle backspaces.
				case (KEY_BACKSPACE):
				case (0x7F):
				case ('\b'):
				{
					// Only backspace if we are after first char.
					if (pos > 0)
					{
						// Clear the old char.
						mvwaddch(cwptr, 0, startpos + pos - 2, ' ' | A_INVIS);
						buf[pos] = '\0';

						// Move cursor.
						--pos;
						wmove(cwptr, 0, startpos + pos - 1);
						wrefresh(cwptr);
					}
					else
					{
						buf[0] = '\0';
					}
					bksp = true;
				}
			}

			// Check to make sure we aren't out of bounds.
			if (pos < maxlen && !bksp)
			{
				// Echo the char.
				wechochar(cwptr, is_pass ? '*' : ch);

				// Change buffer.
				buf[pos] = (char)ch;
				buf[pos + 1] = '\0';

				// Move cursor.
				++pos;
			}
		}
	};

	// Write the username string, and get user input.
	wmove(cwptr, 0, 0);
	wrefresh(cwptr);
	waddstr(cwptr, "Username: ");
	l_get_str(user, false, 9, sizeof("Username: "));

	// If length is zero, just abort.
	if (strlen(user) == 0)
	{
		goto cred_end;
	}

	// Write password, string, then get user input.
	wmove(cwptr, 0, 0);
	wclear(cwptr);
	wrefresh(cwptr);
	waddstr(cwptr, "Password: ");
	l_get_str(pass, true, 63, sizeof("Password: "));

	// If length is zero, just abort.
	if (strlen(pass) == 0)
	{
		goto cred_end;
	}

cred_end:
	// Clear the window.
	wclear(cwptr);
	wrefresh(cwptr);

	curs_set(COH_WND_CURSOR_HIDDEN);
	return true;
}
