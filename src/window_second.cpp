
/*
 * window_second.cpp
 * Implementations of window_second.h methods.
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
#include "window_second.h"
#include "wnd_manager.h"
#include "wnd_string.h"

// Construct window.
window_second::window_second(const vec2& s, const vec2& p, anchor a, const vec4& pad)
	: window(s, p, a, pad), size_orig(s)
{}

// Destructor.
window_second::~window_second()
{}

// Redraw the window.
void window_second::redraw(void)
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

	// Get our vector.
	wnd_manager& wm = wnd_manager::get();
	tt_day* date_info = get_date_info();
	if (date_info)
	{
		std::vector<tt_period>& events = date_info->events;

		// If we have no events, set the status string to say so.
		// If not, then hide it.
		if (events.size())
		{
			components[wm.get_wevnt_str_status()].content = "";
		}
		else
		{
			components[wm.get_wevnt_str_status()].content = COH_SZ_NOEVENTS;
		}

		// Draw the string we just updated.
		redraw_main(w, h);

		// Draw the events list.
		redraw_events(events);
	}
	else
	{
		components[wm.get_wevnt_str_status()].content = COH_SZ_NOEVENTS;
		redraw_main(w, h);
	}

	// Finish draw.
	redraw_end();
}

// Called on resize.
void window_second::on_resize(void)
{
	// Resize the actual window.
	vec2 size_real = calc_real_size();
	size.w = get_main_area_width();
	wresize(wnd, size_real.h, size.w);

	// Call super class method.
	window::on_resize();
}


// Redraw the events.
void window_second::redraw_events(const std::vector<tt_period>& events)
{
	// Move to the start point for text.
	wmove(wnd, COH_WND_MAIN_TITLE_OFFSY + 2, 0);

	// Iterate over all the events.
	for (unsigned i = 0; i < events.size(); ++i)
	{
		// This event
		const tt_period& e = events[i];

		// Draw the bullet.
		waddch(wnd, ACS_BULLET);
		waddch(wnd, ' ');

		// Draw the title with time.
		std::string title;
		title.reserve(26);

		// Don't show the time if they are "placeholder" times. (00:00 to 01:00)
		if (!(e.begin.hour == 0 && e.end.hour == 1))
		{
			char time_str[16];
			sprintf(time_str, "%02u:%02u - %02u:%02u: ", e.begin.hour, e.begin.minute, e.end.hour, e.end.minute);
			title += time_str;
		}
		title += e.title;
		waddstr(wnd, title.c_str());

		// Move cursor down a line, and to the beginning.
		int curs_y, curs_x;
		getyx(wnd, curs_y, curs_x);
		wmove(wnd, curs_y + 2, 0);
	}
}

// Get the date info structure from the main window. No point having it in memory twice here.
tt_day* const window_second::get_date_info(void) const
{
	wnd_manager& wm = wnd_manager::get();
	window_main* w = ((window_main*)wm.get_wnd(COH_WND_IDX_MAIN));
	if (w)
	{
		return w->get_date_info();
	}
	return nullptr;
}

// Get the width of this window.
unsigned window_second::get_main_area_width(void) const
{
	if (COLS > size_orig.w * 2)
	{
		return (unsigned)size_orig.w;
	}
	return (unsigned)COLS / 2 - 3;
}
