#ifndef COH_WND_MANAGER_H
#define COH_WND_MANAGER_H

/*
 * wnd_manager.h
 * - Manages all of the program's TUI !
 * - Literally all ncurses-related stuff
 *   will be handled by this class somehow.
 *
 * - There will be three main "windows":
 *   + Header/title. Should only really get updated once.
 *   + Schedule, where schedule displayed. Main updates happen here.
 *   + Footer, where we show the program state.
 */

// Cursor state defines.
#define COH_WND_CURSOR_HIDDEN  0
#define COH_WND_CURSOR_VISIBLE 1

// Minimum window dimensions. We don't draw
// if we're smaller than this to prevent ugly errors
// and crazy bugs.
#define COH_WND_MIN_WIDTH  36
#define COH_WND_MIN_HEIGHT 15

class application;
class window;
class window_main;
struct datetime_dmy;
struct tt_day;

class wnd_manager
{
public:
	// Singleton
	static wnd_manager& get()
	{
		static wnd_manager inst;
		return inst;
	}

	// Update loop.
	bool update(void);

	// Force redraw everything
	void redraw(void);

	// The initial redraw.
	void redraw_initial(void);

	// Calld on terminal resize.
	void on_resize(void);

	// View a date.
	void view_date(const tt_day&);

	// Set the application pointer.
	inline void set_app(application* const a)
	{
		app = a;
	}

	// Show the prompt to set up the program.
	void show_setup_prompt(void);

	// Get a window from the vector.
	inline window* const get_wnd(int w) const
	{
		return wnds[w];
	}

	// Shorthand to get the main window from vector.
	inline window_main* const get_wnd_main(void) const
	{
		return (window_main*)wnds[COH_WND_IDX_MAIN];
	}

    // Is the window big enough to draw in?
    inline static bool can_draw(void)
    {
        return COLS > COH_WND_MIN_WIDTH && LINES > COH_WND_MIN_HEIGHT;
    }

	// Get indices of window components.
	inline int get_wfoot_str_status(void) const { return wfoot_str_status; }
	inline int get_wfoot_str_retrv (void) const { return wfoot_str_retrv;  }
	inline int get_wmain_str_load  (void) const { return wmain_str_load;   }
	inline int get_wevnt_str_status(void) const { return wevnt_str_status; }
	inline int get_wstat_str_status(void) const { return wstat_str_status; }

	// Our callbacks
	static void cb_login_status_changed(int);
	static void cb_date_set(const datetime_dmy&);

private:
	// Our ncurses windows, and their indices.
	std::vector<window*> wnds;

	// Index of strings we might change.
	int wfoot_str_status; // Status.
	int wfoot_str_retrv;  // Retreival
	int wmain_str_load;   // Loading string.
	int wevnt_str_status; // Events status.
	int wstat_str_status; // Status line status.

	// The application pointer which we can refer to.
	application* app;

private:
	wnd_manager();
	wnd_manager(const wnd_manager&);
	wnd_manager& operator=(const wnd_manager&);
	~wnd_manager();

	// Ncurses-specific methods.
	void ncurses_init  (void);
	void ncurses_deinit(void);

	// Navigation
	void refresh_from_cache(void);
	void refresh_from_server(void);

	// Misc
	bool get_credentials(char*, char*);
};

#endif
