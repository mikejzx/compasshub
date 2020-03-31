#ifndef COH_WINDOW_MAIN_H
#define COH_WINDOW_MAIN_H

/*
 * window_main.h
 * A window which contains the actual schedule.
 * We need this as the main window has specific-code
 * to draw each period.
 *
 * Tiles are drawn with the following behaviour:
 * - Same width as the window.
 * - Height should adjust based on how long period is.
 * - Should not overlap with other tiles.
 * - Colour change based on state of period.
 * - Most periods should be sitting on top of each other.
 *   They should not if they start/finish at different times.
 */

#include "vec4.h"

class  window;
enum   anchor : unsigned char;
enum   period_state : char;
struct datetime_dmy;
struct tt_day;

class window_main : public window
{
public:
	window_main(const vec2&, const vec2&, anchor a=(anchor)(anchor::TOP|anchor::LEFT), const vec4& pad={ 0, 0, 0, 0});
	~window_main();

	// Redraw the tiles on the window.
	void redraw(void) override;

	// On resize.
	void on_resize(void) override;

	// Set the date we are showing.
	void set_date(const datetime_dmy&);

	// Set the date info, and redraw.
	void set_date_info(const tt_day& t);

	// Get the date info.
	inline tt_day* get_date_info(void) const
	{
		return date_info;
	}

private:
	char* date_str;
	tt_day* date_info;

private:
	void redraw_periods(void);
	void draw_fancy_box(unsigned, unsigned, unsigned, int);
	unsigned get_day_length(unsigned*) const;
	int get_state_colours(const period_state) const;
	unsigned get_main_area_width(void) const;
};

#endif
