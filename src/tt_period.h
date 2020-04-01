#ifndef TT_PERIOD_H
#define TT_PERIOD_H

/*
 * tt_period.h
 * Stores information about a single period of
 * the timetable.
 */

#include "time_of_day.h"

struct prefs;

enum period_state : char
{
	NONE        = 0, // Unknown period state.
	NORMAL      = 1, // Regular period.
	CHANGED     = 2, // Irregular period.
	CANCELLED   = 3, // Cancelled period.
	EVENT       = 4, // Not an actual period. Is an event.

	// TODO: Implement these by parsing
	// title for <strike> tags.
	//ROOMCHANGE = 2, // Period has had a room change.
	//SUBSTITUTE = 4  // Substitute teacher.
};

struct tt_period
{
	// Title of the period.
	std::string title;
	time_of_day begin;
	time_of_day end;
	period_state state;

	// Used for constructing pretty titles.
	bool title_parsed;
	std::string t_subj;
	std::string t_room;
	std::string t_tchr;

	// Constructor.
	tt_period(const std::string&, const time_of_day&,
			const time_of_day&, const period_state, const prefs&);
};

#endif
