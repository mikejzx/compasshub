#ifndef COH_WND_STRING_H
#define COH_WND_STRING_H

/*
 * wnd_string.h
 * Represents a string object in a window.
 */

enum anchor : unsigned char;
struct vec2;

struct wnd_string
{
	std::string content;
	anchor anch;
	vec2 pos;
	int attrib;

	wnd_string(const std::string& c, anchor anch, const vec2& p, int at) 
		: content(c), anch(anch), pos(p), attrib(at)  {}
};

#endif
