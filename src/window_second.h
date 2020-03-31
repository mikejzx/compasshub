#ifndef COH_WINDOW_SECOND_H
#define COH_WINDOW_SECOND_H

/*
 * window_second.h
 * This window contains information about events.
 * Didn't call this window_events because that would
 * just look confusing as hell.
 *
 * We simply draw events a "bulleted" list.
 */

#include "vec4.h"

class window;
enum  anchor : unsigned char;
struct tt_period;

class window_second : public window
{
public:
	// Constructors we need.
	window_second(const vec2&, const vec2&, anchor a=(anchor)(anchor::TOP|anchor::LEFT), const vec4& pad={ 0, 0, 0, 0 });
	~window_second();

	// Redraw the window.
	void redraw(void) override;

	// On resize.
	void on_resize(void) override;

private:
	vec2 size_orig;

private:
	tt_day* const get_date_info(void) const;
	void redraw_events(const std::vector<tt_period>&);
	unsigned get_main_area_width(void) const;
};

#endif
