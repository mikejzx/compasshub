
/*
 * window_main.cpp
 * Implementations of window_main.h methods.
 */

#include "pch.h"
#include "anchor.h"
#include "datetime.h"
#include "datetime_dmy.h"
#include "time_of_day.h"
#include "tt_day.h"
#include "tt_period.h"
#include "vec2.h"
#include "window.h"
#include "window_main.h"
#include "wnd_manager.h"
#include "wnd_string.h"

// Construct the window.
window_main::window_main(const vec2& s, const vec2& p, anchor a, const vec4& pad)
	: window(s, p, a, pad), date_info(0)
{
	// Allocate our date string.
	date_str = new char[12];
	strcpy(date_str, "...");

	// Run a quick resize.
	vec2 size_real = calc_real_size();
	unsigned w = get_main_area_width();
	wresize(wnd, size_real.h, w);
}

// Destructor.
window_main::~window_main()
{
	delete date_str;
	if (date_info) { delete date_info; }
}

// Redraw the window.
void window_main::redraw(void)
{
	// Don't redraw if we don't need to.
	if (!dirty)
	{
		return;
	}
	dirty = false;

	// Call the base method.
	static int w, h;
	redraw_begin(&w, &h);
	w = get_main_area_width();
	redraw_main(w, h);

	// Redraw the date str, using extended chars.
	{
		static const int y = COH_WND_MAIN_TITLE_OFFSY;
		size_t tlen = strlen(date_str);

		// Calculate X pos. It's anchored to top-centre.
		int x = w / 2 - tlen / 2;

		// Draw the string and two arrows.
		mvwaddch (wnd, y, x - 2,        ACS_LARROW);
		mvwaddch (wnd, y, x + 1 + tlen, ACS_RARROW);
		mvwaddstr(wnd, y, x, date_str);
	}

	// Redraw the date info if we have it.
	redraw_periods();

	// End the redraw.
	redraw_end();
}

// Called on resize.
void window_main::on_resize(void)
{
	// Resize the actual window.
	vec2 size_real = calc_real_size();
	unsigned w = get_main_area_width();
	wresize(wnd, size_real.h, w);

	// Call super class method.
	window::on_resize();
}

// Set the date.
void window_main::set_date(const datetime_dmy& d)
{
	// Set the date title string.
	// TODO: Preference to change from '.' to '/' or '-', or format, etc.
	sprintf(date_str, "%s, %02u.%02u.%02u", util::get_day_of_week_str(d.dow).c_str(), d.day, d.month, d.year);

	// Since we set a new date, we need to zero everything in the date info to make sure it doesn't
	// get redrawn with old info.
	if (date_info) { delete date_info; }
	date_info = nullptr;

	// We need a redraw. This will not actually draw the schedule yet, just the
	// title.
	window::invalidate();

	// Redraw events too.
	wnd_manager::get().get_wnd(COH_WND_IDX_EVENTS)->invalidate();
}

// Set the date info, allowing a redraw of the timetable.
void window_main::set_date_info(const tt_day& t)
{
	date_info = new tt_day(t);

	// We need a redraw.
	window::invalidate();

	// Tell window manager to redraw events too.
	wnd_manager::get().get_wnd(COH_WND_IDX_EVENTS)->invalidate();
}

// Draw the periods. This will be fun...
void window_main::redraw_periods(void)
{
	// Return if nothing to draw.
	if (!date_info) { return; }

	//LOG_DBUG("\033[00;92m-- -- -- BEGIN TILE DRAW -- -- --\033[00;39m");

	// Variables we might use pretty often.
	static unsigned i;
	static unsigned day_begin;

	// How much space we have in the window
	// with offset taken into account.
	unsigned height_total = LINES - COH_WND_MAIN_PERIODS_OFFSX - 1;

	// Get the approximate total length of the day.
	unsigned day_hours = get_day_length(&day_begin);

	// Forced offset of tiles. Is adjusted if a tile was was too small and needed
	// to be clamped.
	unsigned y_force_next = 0;


	// Draw each of the tiles.
	// The way they should be laid out is defined at the top of
	// window_main.h.
	// Bit of a difficult logic problem to solve...
	for (i = 0; i < date_info->periods.size(); ++i)
	{
		// This period reference.
		tt_period& p = date_info->periods[i];

		// New idea: Convert the begin/finish times into "row space".
		// We ceil the end rows, and floor the beginning.
		float p_beg = p.begin.hour * 60.0f + p.begin.minute;
		p_beg = (p_beg / 60.0f - (float)day_begin) / day_hours;
		float p_end = p.end.hour * 60.0f + p.end.minute;
		p_end = (p_end / 60.0f - (float)day_begin) / day_hours;

		// Calculate rows.
		unsigned p_beg_row = (unsigned)std::floor(p_beg * (float)height_total);
		unsigned p_end_row = (unsigned)std::ceil (p_end * (float)height_total) - 1;

		// Use the row start/ends to calculate size of the tile.
		float h_unclamped = (float)p_end_row - p_beg_row;
		unsigned h = (unsigned)std::max(h_unclamped, 3.0f);

		// Push the next tiles down based on how much more room we need here.
		unsigned y_force = y_force_next;
		y_force_next += (float)h - h_unclamped;

		// Enable the box colour palette.
		int state_col = get_state_colours(p.state);
		wattron(wnd, COLOR_PAIR(state_col));

		// Calculate width.
		unsigned wid = get_main_area_width();

		// Draw our background box.
		draw_fancy_box(p_beg_row + y_force, h, wid, state_col);

		// Draw the label.
		unsigned str_y = COH_WND_MAIN_PERIODS_OFFSY + p_beg_row + 1 + y_force;
		unsigned str_x = COH_WND_MAIN_PERIODS_OFFSX + 1;

		// Create the title string.
		std::string title_str = "";
		title_str.reserve(32);
		if (p.state == period_state::CANCELLED)
		{
			title_str += "(Cancel) ";
		}

		// If we have parsed the period, we can split the information
		// onto multiple lines/sections.
		if (p.title_parsed)
		{
			// Always have subject on first line.
			title_str += p.t_subj;

			// Look for room/teacher changes.
			// The strings will have a forward slash if they are "modified".
			size_t cdelim_room = p.t_room.find("/");
			size_t cdelim_tchr = p.t_tchr.find("/");
			bool chg_room = cdelim_room != std::string::npos;
			bool chg_tchr = cdelim_tchr != std::string::npos;
			std::string str_orig_room, str_new_room = p.t_room;
			std::string str_orig_tchr, str_new_tchr = p.t_tchr;
			if (chg_room)
			{
				str_orig_room = p.t_room.substr(0, cdelim_room);
				str_new_room  = p.t_room.substr(cdelim_room + 1, p.t_room.length() - cdelim_room);
				LOG_DBUG("ROOM CHANGE: '%s' -> '%s'", str_orig_room.c_str(), str_new_room.c_str());
			}
			if (chg_tchr)
			{
				str_orig_tchr = p.t_tchr.substr(0, cdelim_tchr);
				str_new_tchr  = p.t_tchr.substr(cdelim_tchr + 1, p.t_tchr.length() - cdelim_tchr);
				LOG_DBUG("ROOM CHANGE: '%s' -> '%s'", str_orig_tchr.c_str(), str_new_tchr.c_str());
			}

			// Make sure cursor is moved to correct pos before calling this.
			auto l_draw_orig_and_new = [&](const std::string& s_o, const std::string& s_n)
			{
				// Just draw new in standout, and old in dim.
				wattron (wnd, A_STANDOUT);
				waddstr (wnd, s_n.c_str());
				wattroff(wnd, A_STANDOUT);
				waddch  (wnd, ' ');
				wattron (wnd, A_DIM);
				waddstr (wnd, ("(was " + s_o + ")").c_str());
				wattroff(wnd, A_DIM);
			};

			// Adjust based on row count.
			if (h == 3)
			{
				// Single row:
				title_str += ": " + str_new_room + ", " + str_new_tchr;
				mvwaddstr(wnd, str_y, str_x, title_str.c_str());
			}
			else if (h == 4)
			{
				// Two rows.

				// Draw the room.
				if (!chg_room)
				{
					// Not changed.  Just draw normally.
					title_str += " in " + str_new_room;
					mvwaddstr(wnd, str_y, str_x, title_str.c_str());
				}
				else
				{
					// Draw the main title and room.
					title_str += " in ";
					mvwaddstr(wnd, str_y, str_x, title_str.c_str());
					l_draw_orig_and_new(str_orig_room, str_new_room);
				}

				// Draw teacher.
				mvwaddstr(wnd, str_y + 1, str_x, " + Teacher: ");
				if (!chg_tchr)
				{
					waddstr(wnd, str_new_tchr.c_str());
				}
				else
				{
					l_draw_orig_and_new(str_orig_tchr, str_new_tchr);
				}
			}
			else if (h > 4)
			{
				// Three rows.
				mvwaddstr(wnd, str_y, str_x, title_str.c_str());

				// Draw room:
				mvwaddstr(wnd, str_y + 1, str_x, " + In ");
				if (!chg_room)
				{
					waddstr(wnd, str_new_room.c_str());
				}
				else
				{
					l_draw_orig_and_new(str_orig_room, str_new_room);
				}

				// Draw teacher.
				mvwaddstr(wnd, str_y + 2, str_x, " + Teacher: ");
				if (!chg_tchr)
				{
					waddstr(wnd, str_new_tchr.c_str());
				}
				else
				{
					l_draw_orig_and_new(str_orig_tchr, str_new_tchr);
				}
			}
		}
		else
		{
			// Not parsed. Just use the title Compass gives us.
			title_str += p.title;
			mvwaddstr(wnd, str_y, str_x, title_str.c_str());
		}

		// End time label. Anchored to right of the tile.
		char end_time_str[16] = "Finish ";
		char end_time_str_time[6];
		p.end.str(end_time_str_time);
		strcat(end_time_str, end_time_str_time);
		mvwaddstr(wnd, str_y, wid - strlen(end_time_str) - 1, end_time_str);

		// Disable box colour palette
		wattroff(wnd, COLOR_PAIR(state_col));

		// Draw the start time label. XX:XX (6 chars w/ NT char)
		// TODO: 12-hour time preference for normal people.
		char beg_time_str[6];
		p.begin.str(beg_time_str);
		mvwaddstr(wnd, str_y, str_x - 7, beg_time_str);
	}

	//LOG_DBUG("\033[00;92m-- -- -- END TILE DRAW -- -- --\033[00;39m");
}

// Draw a fancy box. Width is same as window.
// NOTE: we cannot have a stretched X width!
void window_main::draw_fancy_box(unsigned ypos, unsigned h, unsigned w, int state_col)
{
	char rcprops;
	chtype c;

	// Draw the box.
	for (unsigned y = 0; y < h; ++y)
	{
		for (unsigned x = COH_WND_MAIN_PERIODS_OFFSX; x < w; ++x)
		{
			// The char we want to print.
			c = ' ' | A_INVIS;

			// Bunch of state booleans packed into single char.
			rcprops =
				  (y == 0)
				| ((y == h - 1) << 1)
				| ((x == COH_WND_MAIN_PERIODS_OFFSX) << 2)
				| ((x == w - 1) << 3);

			// Macros to access row properties.
		#define row_top (rcprops & 1)
		#define row_bot (rcprops & (1 << 1))
		#define col_lft (rcprops & (1 << 2))
		#define col_rgt (rcprops & (1 << 3))

			// Get line graphic chars.
			if (row_top || row_bot)
			{
				if (col_lft)
				{
					// Upper/lower-left corners.
					if (row_top) { c = ACS_ULCORNER; }
					if (row_bot) { c = ACS_LLCORNER; }
				}
				else if (col_rgt)
				{
					// Upper/lower-right corners.
					if (row_top) { c = ACS_URCORNER; }
					if (row_bot) { c = ACS_LRCORNER; }

					// Blacken foreground.
					c |= COLOR_PAIR(state_col + COH_COL_PERIOD_FG_STRIDE);
				}
				else
				{
					// Draw horizontal line on first and last rows that aren't corners.
					c = ACS_HLINE;
					if (row_bot)
					{
						c |= COLOR_PAIR(state_col + COH_COL_PERIOD_FG_STRIDE);
					}
				}
			}
			else if (col_lft)
			{
				// Draw vertical line on first column.
				c = ACS_VLINE;
			}
			else if (col_rgt)
			{
				// Draw vertical line on last column.
				c = ACS_VLINE | COLOR_PAIR(state_col + COH_COL_PERIOD_FG_STRIDE);
			}

			// Just draw invisible chars with background..
			mvwaddch(wnd, COH_WND_MAIN_PERIODS_OFFSY + y + ypos, x, c);

			// Undefine these macros
		#undef row_top
		#undef row_bot
		#undef col_lft
		#undef col_rgt
		}
	}
}

// Gets the length of the day in hours.
// Pretty approximate, we don't take into account the
// minutes.
unsigned window_main::get_day_length(unsigned* day_begin) const
{
	// Lowest/highest. Initialise with crazy values that are
	// guaranteed to be lost to.
	unsigned lo = 24;
	unsigned hi = 0;

	// Iterate over all the periods and find the lowest/highest times..
	for (unsigned i = 0; i < date_info->periods.size(); ++i)
	{
		tt_period& p = date_info->periods[i];

		// Check if this period begins earlier than lowest,
		// or finishes later than highest.
		if (p.begin.hour < lo)
		{
			lo = p.begin.hour;
		}
		else if (p.end.hour > hi)
		{
			hi = p.end.hour;
		}
	}

	// Return the beginning of day.
	*day_begin = lo;

	// Return the difference between the lowest/highest.
	// Just add 1 hour to account for lost minutes.
	return hi - lo + 1;
}

// Get colour from the period state.
int window_main::get_state_colours(const period_state state) const
{
	switch(state)
	{
		// Normal period.
		case (period_state::NORMAL):
		{
			return COH_COL_PERIOD_NORMAL;
		}

		// Changed period. (Subst or room change.)
		case (period_state::CHANGED):
		{
			return COH_COL_PERIOD_RCHANG;
		}

		// Cancelled period.
		case (period_state::CANCELLED):
		{
			return COH_COL_PERIOD_CANCEL;
		}

		// Unknown.
		default:
		{
			LOG_WARN("Unknown period state. Drawing with normal colour...");
			return -1;
		}
	}
}

// Get the width that the window should be.
unsigned window_main::get_main_area_width(void) const
{
	// We return the normal width if columns are greater than threshold.
	if (COLS > size.w * 2)
	{
		return (unsigned)size.w;
	}
	return (unsigned)COLS / 2;
}
