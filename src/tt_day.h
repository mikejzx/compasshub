#ifndef TT_DAY_H
#define TT_DAY_H

/*
 * tt_day.h
 * A day of periods
 */

struct datetime;
struct tt_period;

struct tt_day
{
	std::vector<tt_period> periods;
	std::vector<tt_period> events;
	datetime retrieved;

	// Constructor.
	tt_day()
		: retrieved(datetime())
	{
		periods.reserve(4);
		events.reserve(2);
	}
};

#endif
