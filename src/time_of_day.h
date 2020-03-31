#ifndef COH_TIME_OF_DAY_H
#define COH_TIME_OF_DAY_H

/*
 * time_of_day.h
 * This is used for times which are only based around a single day.
 * Only hours and minutes. Use when this level of minimalism is necessary.
 * (i.e: period begin/end times)
 */

// Helpers - indicates which is larger.
#define S_LEFT -1
#define S_RIGHT 1
#define S_EQUAL 0

struct time_of_day
{
	unsigned hour;
	unsigned minute;

	// Constructor.
	time_of_day(unsigned h, unsigned m)
		: hour(h), minute(m)
	{}

	// Get a nice string into the buffer.
	void str(char* buf) const
	{
		sprintf(buf, "%02u:%02u", hour, minute);
	}

	// Operator overloads.
	inline bool operator<(const time_of_day& other) const
	{
		return tod_cmp(*this, other) == S_RIGHT;
	}
	inline bool operator<=(const time_of_day& other) const
	{
		return tod_cmp(*this, other) >= S_EQUAL;
	}
	inline bool operator>(const time_of_day& other) const
	{
		return tod_cmp(*this, other) == S_LEFT;
	}
	inline bool operator>=(const time_of_day& other) const
	{
		return tod_cmp(*this, other) <= S_EQUAL;
	}
	inline bool operator==(const time_of_day& other) const
	{
		return tod_cmp(*this, other) == S_EQUAL;
	}

private:

	// Compare two time_of_days.
	//  < 0 -> left is larger.
	// == 0 -> they are equal
	//  > 0 -> right is larger
	inline int tod_cmp(const time_of_day& l, const time_of_day& r) const
	{
		// Check the hours.
		if (l.hour < r.hour)
		{
			// Right is larger
			return S_RIGHT;
		}
		if (l.hour > r.hour)
		{
			return S_LEFT;
		}

		// Hours are equal, now check the minutes.
		if (l.hour == r.hour)
		{
			if (l.minute < r.minute)
			{
				return S_RIGHT;
			}
			if (l.minute > r.minute)
			{
				return S_LEFT;
			}
		}

		// They are equal
		return S_EQUAL;
	}
};

// Undefine helpers
#undef S_LEFT
#undef S_RIGHT

#endif
