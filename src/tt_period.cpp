
/*
 * tt_period.cpp
 * Provides implementations of tt_period.h methods.
 */

#include "pch.h"
#include "tt_parser.h"
#include "tt_period.h"

// Constructor.
tt_period::tt_period(const std::string& ttl, const time_of_day& be,
		const time_of_day& en, const period_state s, const prefs& pref)
	: title(ttl), begin(be), end(en), state(s), title_parsed(false)
{
	// Parse the period title if not an event.
	if (state != period_state::EVENT)
	{
	    title_parsed = tt_parser::parse_tt_period_title(title, pref, t_subj, t_room, t_tchr);
	}
}
