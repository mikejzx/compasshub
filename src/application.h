#ifndef COH_APPLICATION_H
#define COH_APPLICATION_H

/*
 * Used as a general manager for the application.
 */

#include "prefs.h"
#include "datetime_dmy.h"

struct datetime_dmy;
struct net_client;
struct tt_day;

class application
{
public:
	application(
		void(*cb_dset)(const datetime_dmy&)
	);
	~application();

	// Getters
	inline prefs get_prefs() const { return preferences; }

	// Check for preferences file to load from.
	bool prefs_check();

	// Create the preferences file using info.
	void prefs_setup(const prefs& p);

	// Set the client to c
	void client_set(net_client* const c);

	// Retrieve timetable for the day. Doesn't read from cache.
	bool get_tt_for_day_update(tt_day& outp, const datetime_dmy& d);

	// Gets the timetable data for day *from cache* if we have it.
	// If not, we return false.
	bool get_tt_for_day_if_cached(tt_day& outp, const datetime_dmy& d) const;

	// Add to the current date.
	void cur_date_add(int);
	inline void cur_date_incr(void) { cur_date_add( 1); }
	inline void cur_date_decr(void) { cur_date_add(-1); }

	// Set the cur_date.
	inline void set_cur_date(const datetime_dmy& d)
	{
		cur_date = d;
		on_set_date(cur_date);
	}

	// Get the cur_date.
	inline datetime_dmy get_cur_date(void) const
	{
		return cur_date;
	}

private:
	// Our prefs object.
	prefs preferences;

	// The client used to get info.
	net_client* client;

	// The current date we are viewing.
	datetime_dmy cur_date;

	// Callbacks.
	void(*on_set_date)(const datetime_dmy&);

	// Whether we can use filesystem caching or not.
	bool cache_enabled;

private:
	// Initialise the cache.
	void cache_init(void);

	// Writes the passed day to cache on disk.
	void cache_write(const tt_day&, int);

	// Actually write the data to file.
	void cache_write_to_file(std::ofstream&, const tt_day&) const;

	// Read from disk cache.
	bool cache_read(tt_day&, int) const;
};

#endif
