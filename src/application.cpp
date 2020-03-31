
/*
 * Provides definitions for application.h methods, etc.
 * We have a static member (tt_cache) in this translation unit,
 * as if we put it in the class we'd need to include a bunch of
 * headers in application.h due to unordered_map template nature.
 */

#include "pch.h"
#include "application.h"
#include "datetime.h"
#include "datetime_dmy.h"
#include "net_client.h"
#include "prefs.h"
#include "tt_day.h"
#include "tt_period.h"

// We only need the cache for this translation unit.
static std::unordered_map<unsigned, tt_day> tt_cache;

// Constructor.
application::application(
	void(*cb_dset)(const datetime_dmy&))
{
	LOG_INFO("Initialising application...");
	on_set_date = cb_dset;

	// Initialise the cache.
	cache_init();
}

// Destructor.
application::~application()
{
	LOG_INFO("Deinitialising application.");
}

// Look for the preferences file.
bool application::prefs_check()
{
	// Check if it exists.
	std::ifstream f(COH_PREFS_FILE_PATH);
	if (!f.good())
	{
		// Doesn't exist. We prompt user.
		return false;
	}
	LOG_INFO("Preferences file exists. Trying to use it.");

	if (!std::getline(f, preferences.hostname)   ||
		!std::getline(f, preferences.path_login) ||
		!std::getline(f, preferences.path_auth)  ||
		!std::getline(f, preferences.path_tt)    ||
		!std::getline(f, preferences.path_logoff))
	{
		LOG_ERROR("Failed to read prefs file.");
		return false;
	}

	LOG_INFO("Successful read prefs file.");

	// We read it.
	return true;
}

// Create the prefs file.
void application::prefs_setup(const prefs& p)
{
	// Just write all our strings in order.
	std::ofstream f(COH_PREFS_FILE_PATH);
	f << p.hostname    << std::endl;
	f << p.path_login  << std::endl;
	f << p.path_auth   << std::endl;
	f << p.path_tt     << std::endl;
	f << p.path_logoff << std::endl;
	f.close();

	LOG_INFO("Created prefs file.");

	// Set the internal object to what we passed.
	preferences = p;
}

// Get timetable from cache if we have it.
bool application::get_tt_for_day_if_cached(tt_day& outp, const datetime_dmy& d) const
{
	// Try read from memory cache first.
	int id = datetime_dmy_id(d).id;
	if (tt_cache.find(id) != tt_cache.end())
	{
		outp = tt_cache[id];
		return true;
	}

	// Try read the filesystem cache to see if we have it on disk.
	if (cache_read(outp, id))
	{
		return true;
	}

	// Not cached.
	return false;
}

// Get the timetable for a day.
bool application::get_tt_for_day_update(tt_day& outp, const datetime_dmy& d)
{
	// Get ID of date.
	int id = datetime_dmy_id(d).id;

	// Retrieve the data using client, add to cache, and return it.
	tt_day ret;

#ifdef COH_USE_SAMPLE_DATA
	// Use example data instead of actually retrieving it.
	ret.periods.clear();
	ret.periods.push_back(tt_period("Sample period 2", { 10,   0 }, { 12,  0 }, period_state::NORMAL));
	ret.periods.push_back(tt_period("Sample period 3", { 12,   0 }, { 13, 30 }, period_state::CHANGED));
	ret.periods.push_back(tt_period("Sample period 1", { 9,    0 }, { 10,  0 }, period_state::NORMAL));
	ret.periods.push_back(tt_period("Sample period 4", { 14,  20 }, { 15, 10 }, period_state::NORMAL));
	ret.periods.push_back(tt_period("Sample period 5", { 15,  10 }, { 15, 25 }, period_state::NORMAL));
	ret.periods.push_back(tt_period("Sample period 6", { 15,  20 }, { 15, 50 }, period_state::NORMAL));
	ret.events .push_back(tt_period("Sample event with an extremely long amount of text to test if the text will actually wrap around the way I'd like it to?",    {  7,  20 }, { 15, 50 }, period_state::EVENT));
	ret.events .push_back(tt_period("Sample event with an extremely long amount of text to test if the text will actually wrap around the way I'd like it to?",    {  6,  20 }, { 12, 50 }, period_state::EVENT));
#else
	// Retrieve from the site.
	if (!client->retrieve_data(ret.periods, ret.events, d))
	{
		LOG_ERROR("Was unable to retrieve data.");
		return false;
	}
#endif

	// Sort vectors by begin time.
	static const auto l_vec_sort = [](const tt_period& l, const tt_period& r)
	{
		return l.begin < r.begin;
	};
	std::sort(ret.periods.begin(), ret.periods.end(), l_vec_sort);
	std::sort(ret.events .begin(), ret.events .end(), l_vec_sort);

	tt_cache[id] = ret;
	cache_write(ret, id);
	outp = ret;
	return true;
}

// Set the client.
void application::client_set(net_client* const c)
{
	client = c;
}

// Get the client.
net_client* const application::client_get(void) const
{
	return client;
}

// Add to the current date
// https://stackoverflow.com/questions/2344330/algorithm-to-add-or-subtract-days-from-a-date
void application::cur_date_add(int days)
{
	// We need to convert the datetime_dmy back to a std::tm.
	std::time_t raw;
	time(&raw);
	std::tm t  = *localtime(&raw);
	t.tm_year = cur_date.year - 1900;
	t.tm_mon  = cur_date.month - 1;
	t.tm_mday = cur_date.day;

	// Seconds since start of epoch.
	static const time_t ONE_DAY = 24 * 60 * 60;
	time_t d = mktime(&t) + days * ONE_DAY;

	// Move the new data into cur_date.
	std::tm datenew = *localtime(&d);
	cur_date.year   = datenew.tm_year + 1900;
	cur_date.month  = datenew.tm_mon + 1;
	cur_date.day    = datenew.tm_mday;
	cur_date.dow    = datenew.tm_wday;

	// Date changed.
	on_set_date(cur_date);
}

// Initialise the cache. This will create the directory if doesn't exist.
void application::cache_init(void)
{
	// Check if we already have a cache directory.
	std::ifstream cache_dir(COH_CACHE_DIR);
	cache_enabled = true;
	if (cache_dir.good())
	{
		return;
	}

	LOG_INFO("No cache directory. Creating...");

	// Create the directory.
	if (mkdir(COH_CACHE_DIR, 0777) != 0)
	{
		LOG_WARN("Unable to create cache directory!");
		cache_enabled = false;
		return;
	}
}

// Write a date to the cache.
void application::cache_write(const tt_day& day, int id)
{
	// Return if we don't have a cache directory.
	if (!cache_enabled)
	{
		return;
	}

	// Get the filename.
	char fname[sizeof(COH_CACHE_DIR) + sizeof("YYYYMMDD")];
	if (sprintf(fname, COH_CACHE_DIR "%u", id) == -1)
	{
		LOG_ERROR("Error while getting filename for cache file in cache_write.");
		return;
	}

	// Write to the cache directory.
	std::ofstream file(fname);
	if(!file.good())
	{
		LOG_ERROR("Couldn't open file stream for cache file.");
		return;
	}

	// Write out all the info the file.
	cache_write_to_file(file, day);

	file.close();
}

// Actually write the data to the cache.
void application::cache_write_to_file(std::ofstream& f, const tt_day& d) const
{
	// First write the retrieval time as YYYY-MM-DD HH:MM in UTC.
	std::tm d_rt = *gmtime(&d.retrieved.time_utc);
	char d_rt_str[sizeof "YYYY-MM-DD HH:MM\n"];
	if (sprintf(d_rt_str, COH_CACHE_RETRV_DATE_FORMAT "\n",
		d_rt.tm_year + 1900,
		d_rt.tm_mon + 1,
		d_rt.tm_mday,
		d_rt.tm_hour,
		d_rt.tm_min) < 0
	)
	{
		LOG_ERROR("Didn't write all retrieval datetime info to cache file.");
		return;
	}
	f << d_rt_str;

	// Write number of periods and events.
	f << d.periods.size() << "," << d.events.size() << std::endl;

	// This lambda is used twice, for writing periods and events.
	auto l_write_tt_periods = [&f](const std::vector<tt_period>& data)
	{
		// Iterate over all the periods.
		for (unsigned i = 0; i < data.size(); ++i)
		{
			// Get period reference.
			const tt_period& p = data[i];

			// Allocate for single line of information.
			char* p_str = (char*)alloca(p.title.size()
				+ sizeof("\"\"")
				+ sizeof(COH_CACHE_DELIM "HH:MM") * 2
				+ sizeof(COH_CACHE_DELIM "PST\n"));
			sprintf(
				p_str,
				"%s" COH_CACHE_DELIM
				"%02u:%02u" COH_CACHE_DELIM
				"%02u:%02u" COH_CACHE_DELIM
				"%03u\n",
				p.title.c_str(),
				p.begin.hour, p.begin.minute,
				p.end.hour, p.end.minute,
				(unsigned)p.state
			);

			// Write to file.
			f << p_str;
		}
	};

	// Write the periods and events.
	l_write_tt_periods(d.periods);
	l_write_tt_periods(d.events);
}

// Try read from the cache on disk. If we get it, we will also add it to the memory cache.
bool application::cache_read(tt_day& outp, int id) const
{
	// Get filename. TODO: Move to a method.
	char fname[sizeof(COH_CACHE_DIR) + sizeof("YYYYMMDD")];
	if (sprintf(fname, COH_CACHE_DIR "%u", id) == -1)
	{
		LOG_ERROR("Error while getting filename for cache file in cache_read.");
		return false;
	}

	// Check if file exists.
	std::ifstream f(fname);
	if (!f.good())
	{
		return false;
	}

	// Does exist, lets try read.
	std::string line;
	tt_day o;

	// We use this macro pretty often. Just gets a line.
#define S_LINE_GET if (!std::getline(f, line))\
	{ LOG_WARN("Couldn't read cache file for ID:%d.", id); return false; }

	// First line is the retrieval date in YYYY-MM-DD HH:MM
	S_LINE_GET;
	int or_yr, or_mon, or_day, or_hr, or_mn;
	sscanf(line.c_str(), COH_CACHE_RETRV_DATE_FORMAT,
		&or_yr, &or_mon, &or_day, &or_hr, &or_mn);
	std::tm or_date = { 0 };
	or_date.tm_year = or_yr - 1900;
	or_date.tm_mon  = or_mon - 1;
	or_date.tm_mday = or_day;
	or_date.tm_hour = or_hr; or_date.tm_min  = or_mn;
	o.retrieved = datetime(timegm(&or_date));

	// Second line is number of periods/events.
	S_LINE_GET;
	unsigned size_p, size_e;
	sscanf(line.c_str(), "%u,%u", &size_p, &size_e);

	// Use this lambda for both vectors.
	auto l_get_tt_periods = [&](std::vector<tt_period>& ovec, unsigned size)
	{
		// Get periods.
		for (unsigned i = 0; i < size; ++i)
		{
			S_LINE_GET;

			// Find how long the string in quotation marks is.
			size_t q_1 = line.find(COH_CACHE_DELIM);
			if (q_1 == std::string::npos)
			{
				LOG_WARN("Cache file for date ID:%d contains invalid period title.", id);
				return false;
			}

			// We use these in the sscanf.
			char* p_title = (char*)alloca(q_1 + 1);
			unsigned pb_hr, pb_min, pe_hr, pe_min;
			unsigned p_state;

			// Get the period data.
			// The crazy %[^\t\n] will find anything except tabs and NL chars.
			sscanf(line.c_str(),
				"%[^\t\n" COH_CACHE_DELIM "]" COH_CACHE_DELIM
				"%02u:%02u" COH_CACHE_DELIM
				"%02u:%02u" COH_CACHE_DELIM
				"%03u",
				p_title, &pb_hr, &pb_min, &pe_hr, &pe_min, &p_state
			);

			// Add to the vector.
			ovec.emplace_back(
				p_title,
				time_of_day(pb_hr, pb_min),
				time_of_day(pe_hr, pe_min),
				(period_state)p_state
			);
		}
		return true;
	};

	// Get vectors.
	if (
		!l_get_tt_periods(o.periods, size_p) ||
		!l_get_tt_periods(o.events,  size_e)
	)
	{
		return false;
	}

	// Sort vectors by begin time.
	static const auto l_vec_sort = [](const tt_period& l, const tt_period& r)
	{
		return l.begin < r.begin;
	};
	std::sort(o.periods.begin(), o.periods.end(), l_vec_sort);
	std::sort(o.events .begin(), o.events .end(), l_vec_sort);

	// Assign to our dummy object. Makes a copy though...
	outp = o;

	// Put into our memory cache.
	tt_cache[id] = o;

	return true;
}
