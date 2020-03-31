#ifndef COH_TT_PARSER_H
#define COH_TT_PARSER_H

/*
 * tt_parser.h
 * Contains methods to parse JSON timetable into
 * timetable objects.
 */

struct tt_period;

namespace tt_parser
{
	/*
	 * Used for UTC conversion. Not mean't to be used
	 * outside this file.
	 */
	std::time_t get_epoch_time(const std::string& t, int* status);

	/*
	 * Convert JSON data to a tt_period vector.
	 * (Clears outp buffer before beginning.)
	 */
	bool parse_json(std::vector<tt_period>& outp, std::vector<tt_period>& outp_events, const std::string& inp);
}

#endif
