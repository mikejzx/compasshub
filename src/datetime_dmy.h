#ifndef COH_DATETIME_DMY_H
#define COH_DATETIME_DMY_H

/*
 * Represents only a day/month/year date.
 * No minutes, etc.
 */

struct datetime_dmy
{
	// Day of month, and month.
	uint8_t day, month;

	// Year
	uint16_t year;

	// Day of the week.
	uint8_t dow;

	datetime_dmy()
		: day(0), month(0), year(0), dow(0)
	{}
	datetime_dmy(uint8_t d, uint8_t m, uint16_t y, uint8_t dw)
		: day(d), month(m), year(y), dow(dw)
	{}

	inline bool operator==(const datetime_dmy& other) const
	{
		return (
			day == other.day
			&& month == other.month
			&& year == other.year
		);
	}
};

/*
 * Use this to identify datetime as an integer
 * which we can use as the key of an unordered map.
 */
struct datetime_dmy_id
{
	int id;

	// The ID is basically just the date in number form.
	// For example: 05.12.2020 becomes 20201205
	datetime_dmy_id(datetime_dmy a)
		: id((int)a.day + (int)a.month * 100 + (int)a.year * 10000)
	{}
};

#endif
