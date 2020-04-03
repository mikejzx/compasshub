#ifndef COH_TT_PARSER_H
#define COH_TT_PARSER_H

/*
 * tt_parser.h
 * Contains methods for general parsing in the application.
 * Includes JSON parsing, and period title seperation.
 */

struct tt_period;
struct prefs;

namespace tt_parser
{
	/*
	 * Used for UTC conversion. Not mean't to be used
	 * outside this file.
	 */
	std::time_t get_epoch_time(const std::string&, int*);

	/*
	 * Convert JSON data to a tt_period vector.
	 * (Clears outp buffer before beginning.)
	 */
	bool parse_json(std::vector<tt_period>&, std::vector<tt_period>&, const std::string&, const prefs&);

	/*
	 * Parse information from period title.
	 */
	bool parse_tt_period_title(const std::string&, const prefs&, std::string&, std::string&, std::string&);

	/*
	 * Parse a period title for "modifications". (i.e: room changes.)
	 * These "modifications" are determined merely by HTML <strike> tags.
	 */
	bool parse_tt_period_title_strikeouts(const std::string&, std::string&, std::string&);
}

#endif
