#ifndef COH_VEC2_H
#define COH_VEC2_H

/*
 * vec2.h
 * A basic structure containing two integer components.
 * Used for positions and sizes.
 */

struct vec2
{
	union { int y, h; };
	union { int x, w; };
};

#endif
