
/*
 * window.cpp
 * Implementations of window.h methods.
 */

#include "pch.h"
#include "anchor.h"
#include "vec2.h"
#include "vec4.h"
#include "window.h"
#include "wnd_manager.h"
#include "wnd_string.h"

// Check if anchor Z is type W.
#define S_ANCH_IS(z, w) (z & anchor::w)

// Constructor.
window::window(const vec2& s, const vec2& p, anchor a, const vec4& pad)
	: size(s), size_real({ 0 }), pos(p), padding(pad), anch(a)
{
	// Temporary dimensions and positions.
	size_real = calc_real_size();
	vec2 p_real = calc_real_pos();

	// Create the window.
	wnd = newwin(size_real.h, size_real.w, p_real.y, p_real.x);
}

// Destructor.
window::~window()
{
	// Free the window.
	delwin(wnd);
}

// Add a string to this window.
int window::add_str(const std::string& s, const vec2& p, anchor a, int att)
{
	// Add to the components vector.
	components.emplace_back(s, a, p, att);

	// Return the index.
	return components.size() - 1;
}

// Change the string
void window::chg_str(int idx, const std::string& to, int att)
{
	// Don't do anything if we already have this content.
	if (components[idx].content.compare(to) == 0
		&& att == -1)
	{
		return;
	}

	// Change string content
	components[idx].content = to;

	// Set attributes if it's not -1.
	//if (att != -1)
	//{
		components[idx].attrib = att;
	//}

	// Ask for a redraw.
	invalidate();
}

// Called when window gets resized.
// Make sure to call redraw on window manager after
// calling this.
void window::on_resize(void)
{
	// Move our window to the proper spot.
	size_real   = calc_real_size();
	vec2 p_real = calc_real_pos();
	mvwin(wnd, p_real.y, p_real.x);

	// We need a redraw.
	invalidate();
}

// Begin the redraw. Also calculates with and height
// we need.
void window::redraw_begin(int* w, int* h)
{
	// Clear window
	wclear(wnd);

	// Get width/height.
	*h = (size.y == COH_WND_STRETCH) ? LINES : std::min(size.y, LINES);
	*w = (size.x == COH_WND_STRETCH) ? COLS  : std::min(size.x, COLS );
}

// Redraw the main strings in the window.
void window::redraw_main(int w, int h)
{
	// Iterate over components and draw everything
	for (unsigned i = 0; i < components.size(); ++i)
	{
		wnd_string& c = components[i];
		size_t tlen = c.content.length();

		// Move the cursor based on anchor and position.
		int y, x;

		// Left-side anchor.
		// - x simply becomes the relative x pos.
		if (S_ANCH_IS(c.anch, LEFT))
		{
			x = c.pos.x;
		}

		// Right-side anchor.
		// - x becomes wnd width, minus text length, minus offset.
		if (S_ANCH_IS(c.anch, RIGHT))
		{
			x = w - c.pos.x - tlen;
		}

		// Top anchor.
		// - y becomes relative y pos.
		if (S_ANCH_IS(c.anch, TOP))
		{
			y = c.pos.y;
		}

		// Bottom anchor.
		// - y becomes wnd height, minus offset
		if (S_ANCH_IS(c.anch, BOTTOM))
		{
			y = h - c.pos.y - 1;
		}

		// Centred-vertical anchor.
		// - y becomes wnd height / 2 plus y position.
		if (S_ANCH_IS(c.anch, CENT_V))
		{
			y = h / 2 + c.pos.y;
		}

		// Centred-horizontal anchor.
		// - x becomes wnd width/2 minux textlength/2, plus x pos.
		if (S_ANCH_IS(c.anch, CENT_H))
		{
			x = w / 2 - tlen / 2 + c.pos.x;
		}

		// Move the cursor to our calculated position.
		wmove(wnd, y, x);

		// Enable attributes.
		if (c.attrib != -1)
		{
			wattron(wnd, c.attrib);
		}

		// Draw the string.
		waddstr(wnd, c.content.c_str());

		// Disable attrib.
		if (c.attrib != -1)
		{
			wattroff(wnd, c.attrib);
		}
	}
}

// Redraw the window.
void window::redraw(void)
{
	// Don't redraw if we don't need to.
	if (!dirty)
	{
		return;
	}
	dirty = false;

	int w, h;
	redraw_begin(&w, &h);
	redraw_main(w, h);
	redraw_end();
}

// Set a colour pair to be active.
void window::colour_bg_set(int col)
{
	wbkgd(wnd, COLOR_PAIR(col));
}

// Tell the window manager to redraw.
void window::invalidate(void)
{
	// Mark dirty.
	dirty = true;

	// Tell window manager to redraw.
	wnd_manager::get().redraw();
}

// Calculate the real size of the window.
vec2 window::calc_real_size(void) const
{
	vec2 ret = size;

	// Used for padding.
	int w_add = 0;

	// Stretch along Y.
	if (size.h == COH_WND_STRETCH)
	{
		// Set height to number of lines.
		ret.h = LINES;

		// Apply padding.
		ret.h -= padding.top;
		ret.h -= padding.bottom;
		if (S_ANCH_IS(anch, LEFT))
		{
			w_add -= padding.left;
		}
		if (S_ANCH_IS(anch, RIGHT))
		{
			w_add -= padding.right;
		}
	}

	// Stretch along X.
	if (size.w == COH_WND_STRETCH)
	{
		ret.w = COLS;
	}

	// Add padding
	ret.w += w_add;

	return ret;
}

// Calculate the window position
vec2 window::calc_real_pos(void) const
{
	vec2 ret = pos;

	// Used for padding.
	int x_add = 0;

	if (size.h == COH_WND_STRETCH)
	{
		// Don't allow Y movement if we are stretched along it.
		ret.y = 0;

		// Apply padding.
		ret.y += padding.top;
		if (S_ANCH_IS(anch, LEFT))
		{
			x_add += padding.left;
		}
	}
	else
	{
		// Apply legal anchors for not stretched along Y.
		// - BOTTOM
		// - TOP (No change)
		// Centre is not implemented.

		// Bottom
		// - Y becomes termheight - winheight - Y offset
		if (S_ANCH_IS(anch, BOTTOM))
		{
			ret.y = LINES - size_real.y - pos.y - padding.bottom;
		}
	}

	if (size.w == COH_WND_STRETCH)
	{
		// Don't allow X movement if we are stretched along it.
		ret.x = 0;
	}
	else
	{
		// Apply legal anchors for not stretched along X.
		// - LEFT (No change)
		// - RIGHT
		// Centre is not implemented.

		// Right
		if (S_ANCH_IS(anch, RIGHT))
		{
			ret.x = COLS - size_real.x - pos.x - padding.right;
		}
	}

	// Add padding.
	ret.x += x_add;

    // Clamp values.
    ret.x = std::max(0, ret.x);
    ret.y = std::max(0, ret.y);

	return ret;
}

// Undefine the macro.
#undef S_ANCH_IS
