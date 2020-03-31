#ifndef COH_WINDOW_H
#define COH_WINDOW_H

/*
 * window.h
 * - Wrapper around ncurses window.
 */

#include "wnd_string.h"
#include "vec4.h"

enum anchor : unsigned char;
struct vec2;

class window
{
public:
	window(const vec2&, const vec2&, anchor a=(anchor)(anchor::TOP|anchor::LEFT), const vec4& pad={ 0, 0, 0, 0 });
	virtual ~window();

	// Add a string to the window.
	int add_str(const std::string&, const vec2&, anchor, int a=-1);

	// Change string content.
	void chg_str(int, const std::string&, int a=-1);

	// Redraw the window.
	virtual void redraw(void);

	// Called on a resize.
	virtual void on_resize(void);

	// Colour pair
	void colour_bg_set(int);

	// Ask for a redraw.
	void invalidate(void);

	// Get the window pointer.
	inline WINDOW* const get_wndptr(void) const { return wnd; }

protected:
	// Dimensions of window.
	// We set to -1 for stretch.
	// No padding at the moment since we don't really need.
	vec2 size;

	// The actual size of the window. This will react
	// to resizes.
	vec2 size_real;

	// The position, anchored.
	vec2 pos;

	// The padding for stretched windows.
	vec4 padding;

	// Where the window is anchored to.
	anchor anch;

	// The internal window pointer.
	WINDOW* wnd;

	// All of the components attached to the window.
	// Just strings with anchors and positions.
	std::vector<wnd_string> components;

	// The colour of the window. All text etc uses it.
	int colour;

	// Do we need a redraw?
	bool dirty;

protected:
	// Obviously use these in redrawing.
	// They are here mainly for subclass support. (Main window)
	void redraw_begin(int*, int*);
	void redraw_main(int, int);
	inline void redraw_end(void) { wrefresh(wnd); }

	// May merge these into a single method.
	vec2 calc_real_size(void) const;
	vec2 calc_real_pos(void)  const;
};

#endif
