
#include "pch.h"
#include "prefs.h"
#include "tt_parser.h"
#include "tt_period.h"

// Get epoch time
std::time_t tt_parser::get_epoch_time(const std::string& t, int* status)
{
	// UTC format.
	static const char* date_format = "%Y-%m-%dT%H:%M:%SZ";

	std::istringstream ss { t };
	std::tm dt;
	ss >> std::get_time(&dt, date_format);
	if (ss.fail())
	{
		*status = 0;
		return (std::time_t)0;
	}
	*status = 1;
	return timegm(&dt);
}

// Parse JSON to tt_period vector of periods, and events.
bool tt_parser::parse_json(
	std::vector<tt_period>& outp,
	std::vector<tt_period>& outp_events,
	const std::string& inp,
	const prefs& pref
)
{
	outp.clear();
	outp_events.clear();

	using namespace rapidjson;

	// Use this macro for all error-like things.
#define S_JSONASSERT(condition, msg)\
	if (condition)\
	{ LOG_ERROR("[JSON] " msg); return false; }

	// Try to parse the file..
	Document doc;
	S_JSONASSERT(doc.Parse(inp.c_str()).HasParseError(), "Unable to parse JSON file!");
	S_JSONASSERT(!doc.HasMember("d"), "Unable to parse JSON file!");

	// Get root array member and iterate.
	const Value& d = doc["d"];
	for (SizeType i = 0; i < d.Size(); ++i)
	{
		// Skip non-objects.
		if (!d[i].IsObject()) { continue; }

		// Get this main period object.
		auto pobj = d[i].GetObject();

		// Get the data we want.
		S_JSONASSERT(!pobj.HasMember("longTitleWithoutTime") || !pobj["longTitleWithoutTime"].IsString(), "Couldn't get Title.");
		S_JSONASSERT(!pobj.HasMember("backgroundColor"     ) || !pobj["backgroundColor"     ].IsString(), "Couldn't get Background Colour.");
		S_JSONASSERT(!pobj.HasMember("start"               ) || !pobj["start"               ].IsString(), "Couldn't get Start Time");
		S_JSONASSERT(!pobj.HasMember("finish"              ) || !pobj["finish"              ].IsString(), "Couldn't get Finish Time");
		const char* j_title  = (const char*)pobj["longTitleWithoutTime"].GetString();
		const char* j_bgcol  = (const char*)pobj["backgroundColor"     ].GetString();
		const char* j_start  = (const char*)pobj["start"               ].GetString();
		const char* j_finish = (const char*)pobj["finish"              ].GetString();

		// Get state from BG color for now.
		// In future the strikeout tags in long title could be
		//     parsed for more detail.
		// We do this first to make sure we're not wasting any much performance,
		//     since we aren't pushing events to the vector currently.
		// Compass features:
		// - #dce6f4 Normal
		// - #f4dcdc Room change/substitute
		// - #EFEFEF Cancelled
		// - #2951B9 Events (Don't show these in timetable, maybe on side?)
		period_state s = period_state::NORMAL;
		if (strcmp(j_bgcol, "#dce6f4") == 0)
		{
			s = period_state::NORMAL;
		}
		else if (strcmp(j_bgcol, "#EFEFEF") == 0)
		{
			s = period_state::CANCELLED;
		}
		else if (strcmp(j_bgcol, "#2951B9") == 0)
		{
			// Events are considered periods too.
			s = period_state::EVENT;
		}
		else
		{
			s = period_state::CHANGED;
		}

		// Perform UTC conversion.
		// https://stackoverflow.com/questions/42854679/c-convert-given-utc-time-string-to-local-time-zone
		int success = 0;
		std::time_t utc_start  = get_epoch_time(j_start, &success);
		S_JSONASSERT(!success, "Couldn't get epoch Start Time." );
		std::time_t utc_finish = get_epoch_time(j_finish, &success);
		S_JSONASSERT(!success, "Couldn't get epoch Finish Time.");
		std::tm local_start    = *localtime(&utc_start );
		std::tm local_finish   = *localtime(&utc_finish);

		// Check if the start/end time is in school hours. If not, skip this.
		if ((s != period_state::EVENT)
			&& (local_start.tm_hour < 7 || local_start.tm_hour >= 18))
		{
			continue;
		}

		// Don't construct here, just pass the arguments using this macro to emplace.
	#define S_RESULT\
		j_title,\
		time_of_day((unsigned)local_start .tm_hour, (unsigned)local_start .tm_min),\
		time_of_day((unsigned)local_finish.tm_hour, (unsigned)local_finish.tm_min), s, pref

		// Push into the right vector.
		if (s != period_state::EVENT)
		{
			outp.emplace_back(S_RESULT);
		}
		else
		{
			outp_events.emplace_back(S_RESULT);
		}

		// Undefine macro.
	#undef S_RESULT
	}

	// Parse success.
	return true;
}

// Parse the period title for information.
// Returns true if there was success getting all information.
bool tt_parser::parse_tt_period_title(const std::string& title, const prefs& pref,
	std::string& out_subj, std::string& out_room, std::string& out_tchr
)
{
	// Our title for periods is in the format of:
	// '<Unused> - <Subject Name> - <Room name> - <Teacher>'
	char p_subj[64];
	char p_room[64];
	char p_tchr[64];

	// Check if the string is enormous.
	if (title.length() >= sizeof(" - ") * 3 + 64 * 3)
	{
		return false;
	}

	// Get the data.
	if (sscanf(title.c_str(), "%*s - %s - %s - %s", p_subj, p_room, p_tchr) != 3)
	{
		return false;
	}

	// Returns an alias if we have it, and just the input if not.
	auto l_check_alias = [&](const std::string& s)
	{
		// If we have string in the aliases map, return it.
		if (pref.aliases.find(s) != pref.aliases.end())
		{
			return pref.aliases.at(s);
		}
		return s;
	};

	// Check if our data is aliased.
	out_subj = l_check_alias(p_subj);
	out_room = l_check_alias(p_room);
	out_tchr = l_check_alias(p_tchr);

	return true;
}
