/*
 * astrodatetime.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 27-jan-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/astrodatetime.h $
 *
 */

#ifndef ASTRODATETIME_H_
#define ASTRODATETIME_H_

#include <string>
#include <fstream>
#include <QDateTime>
#include "astrotime.h"
#include "astrodate.h"

class AstroDateTime : public AstroTime, public AstroDate
{
public:
	AstroDateTime();
	AstroDateTime(const std::string &);
	AstroDateTime(const QString &);
	AstroDateTime(double mod_julian);
	AstroDateTime(int julian_day, double &time);
	AstroDateTime(const AstroDate &date, const AstroTime &time = AstroTime(0.0));
	AstroDateTime(const QDateTime &);
	AstroDateTime(unsigned short day, unsigned short month, unsigned int year, int hours, short int minutes, short int seconds);
	AstroDateTime(AstroTime const &time);
	AstroDateTime(int julian_day);

	static AstroDateTime UTfromLST(const AstroTime &lstTime, const AstroDate &calendarDate);

	virtual ~AstroDateTime();

	friend QDataStream& operator<< (QDataStream &out, const AstroDateTime &date_time); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, AstroDateTime &date_time); // used for reading binary data from file

	bool operator=(const QString &date_time);
	bool operator=(const double &mod_julian);
	bool operator=(const QDateTime &date_time);
	bool operator==(const AstroDateTime &right) const;
	bool operator>(const AstroDateTime &right) const;
	bool operator<(const AstroDateTime &right) const;
	bool operator>=(const AstroDateTime &right) const;
	bool operator<=(const AstroDateTime &right) const;
	bool operator>(const AstroDate &right) const {return (static_cast<AstroDate>(*this) > right);}
	bool operator<(const AstroDate &right) const {return (static_cast<AstroDate>(*this) < right);}
	bool operator>=(const AstroDate &right) const {return (static_cast<AstroDate>(*this) >= right);}
	bool operator<=(const AstroDate &right) const {return (static_cast<AstroDate>(*this) <= right);}
	bool operator>(const double &right) const {return (this->toJulian() > right);}
	bool operator<(const double &right) const {return (this->toJulian() < right);}
	bool operator>=(const double &right) const {return (this->toJulian() >= right);}
	bool operator<=(const double &right) const {return (this->toJulian() <= right);}
	bool operator!=(const AstroDateTime & right) const;
	AstroDateTime &operator+=(const AstroDateTime &other);
	AstroDateTime &operator+=(const AstroTime &other);
	AstroTime operator-=(const AstroDateTime &other);
	AstroDateTime &operator-=(const AstroTime &other);
	// calculate the total time difference in hhhh:mm:ss (instead of date and time difference)
	AstroTime timeDifference(const AstroDateTime &other) const;
	AstroTime time(void) const {return static_cast<AstroTime>(*this);}
	AstroDate date(void) const {return static_cast<AstroDate>(*this);}
	AstroTime toLST(void) const;
	inline QDateTime toQDateTime(void) const {return QDateTime(toQDate(), toQTime());}

	AstroDateTime addDays(unsigned int days) const;
	AstroDateTime addHours(unsigned int hours) const;
	AstroDateTime addMinutes(unsigned int mins) const;
	AstroDateTime addSeconds(unsigned int seconds) const;
	AstroDateTime addWeeks(unsigned int weeks) const;
	AstroDateTime subtractDays(unsigned int days) const;
	AstroDateTime subtractMinutes(unsigned int mins) const;
	AstroDateTime subtractHours(unsigned int hours) const;
	AstroDateTime subtractWeeks(unsigned int weeks) const;
	AstroDateTime subtractSeconds(unsigned int seconds) const;

	// Get methods
    bool isSet(void) const { if (this->getDate().isSet() || this->getTime().isSet()) return true; else return false; }
    double toJulian(void) const {return (this->get_MJDay() + this->getJulianTime());}
	std::string toString(void) const;
	std::string toSASDateTimeString(void) const;

    // Set methods
    void clear(void) {clearDate(); clearTime();}
	bool setDateTime(const std::string &date_time) {return setDateTime(QString(date_time.c_str()));}
	bool setDateTime(const QString &date_time);
	void setDate(const AstroDate &date);
	void setTime(const AstroTime &time);

private:
	void calculateModifiedJulianDate(void);
	void calculateGregorianDateTime(void);
};

inline AstroDateTime const operator+(AstroDateTime const &l_hand, AstroDateTime const &r_hand)
{
	return AstroDateTime(l_hand) += r_hand;
}

inline AstroDateTime const operator-(AstroDateTime const &l_hand, AstroDateTime const &r_hand)
{
	return AstroDateTime(l_hand) -= r_hand;
}

inline AstroDateTime const operator+(AstroDateTime const &l_hand, AstroTime const &r_hand)
{
	return AstroDateTime(l_hand) += r_hand;
}

inline AstroDateTime const operator-(AstroDateTime const &l_hand, AstroTime const &r_hand)
{
	return AstroDateTime(l_hand) -= r_hand;
}

inline const AstroDateTime &maxdate(const AstroDateTime &first, const AstroDateTime &second) {
	if (first >= second) return first;
	else return second;
}

inline const AstroDateTime &mindate(const AstroDateTime &first, const AstroDateTime &second) {
	if (first <= second) return first;
	else return second;
}

#endif /* ASTRODATETIME_H_ */
