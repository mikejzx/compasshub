#ifndef COH_VEC4_H
#define COH_VEC4_H

/*
 * vec4.h
 * Structure representing four interger components.
 * Used for stretched window padding.
 *
 * Padding example
 * +---------------------------+
 * |          pad top          |
 * |       +--------------+    |
 * |       |anchor: right |pad |
 * |       |stretch y     |rght|
 * |       |              |    |
 * |       +--------------+    |
 * |           pad bot         |
 * +---------------------------+
 * Window pos:
 * + If stretched along Y:
 *     - Add top padding to Y pos.
 *     - Subtract top padding from height.
 *     - Subtract bottom padding from height.
 *     + If LEFT  anchor:
 *         - Add left padding to X pos.
 *         - Subtract left padding from width.
 *     + If RIGHT anchor:
 *         - Subtract right padding from width.
 *         - Pos becomes total width - wnd width.
 *
 * + If stretched along X:
 *     - Not yet needed, so we don't handle.
 */

struct vec4
{
	// Padding is stored as top, right, bottom, left.
	union { int x, top; };
	union { int y, right; };
	union { int z, bottom; };
	union { int w, left; };
};

#endif
