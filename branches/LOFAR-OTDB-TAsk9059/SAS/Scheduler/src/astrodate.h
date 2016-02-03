/*
 * astrodate.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 13, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/astrodate.h $
 *
 */

#ifndef ASTRODATE_H_
#define ASTRODATE_H_

#include <string>
#include <fstream>
#include <QDataStream>
#include <QDate>
#include "lofar_utils.h"

extern const char * DAY_NAMES_ENG[7];
extern const char * DAY_SHORT_NAMES_ENG[7];
extern const char * MONTH_SHORT_NAMES_ENG[12];
extern const char * MONTH_NAMES_ENG[12];

class AstroDate {
public:
	AstroDate();
	virtual ~AstroDate();
	AstroDate(const char *chr);
	AstroDate(const QDate &);
	AstroDate(const QString &);
	AstroDate(const std::string &str);
	AstroDate(unsigned int);
	AstroDate(quint8 day, quint8 month, quint16 year);

	// overloaded operators
	friend QDataStream& operator<< (QDataStream &out, const AstroDate &date); // used for writing binary file
	friend QDataStream& operator>> (QDataStream &in, AstroDate &date); // used for reading binary file

	AstroDate &operator=(const QString &);
	AstroDate &operator=(int modJulian);
	AstroDate &operator++();
	const AstroDate operator++(int);
	AstroDate &operator--();
	const AstroDate operator--(int);
	AstroDate &operator+=(AstroDate const &other);
	AstroDate &operator-=(AstroDate const &other);
	bool operator==(const AstroDate &right) const;
	bool operator!=(const AstroDate & right) const;
	bool operator==(int right) const;
	bool operator>(const AstroDate &right) const;
	bool operator<(const AstroDate &right) const;
	bool operator>=(const AstroDate &right) const;
	bool operator<=(const AstroDate &right) const;

	// Get methods
	inline QDate toQDate(void) const { return QDate(year, month, day); }
	inline bool isSet(void) const { if(mod_julian_day) return true; else return false; }
	const AstroDate &getDate(void) const {return *this;}
	inline quint16 toJulian() const {return mod_julian_day;}
	inline quint16 get_MJDay(void) const {return mod_julian_day;}
    std::string toString(short mode = 0) const;
    inline quint8 getDay() const {return day;}
    inline quint8 getWeek() const {return week;}
    inline quint8 getMonth() const {return month;}
    inline quint16 getYear() const {return year;}
    // get the day of the week number (monday = 0)
    unsigned int getDayOfTheWeek(void) const;
    const char *getDayString(void) const;
	bool isLeapYear(unsigned int year) const;

    // Set methods
    void clearDate(void) {day = 0; week = 0; month = 0; year = 0; mod_julian_day = 0;}
	void setDateStr(const QString &);
	void setDateStr(const std::string &str) {setDateStr(QString(str.c_str()));}
    void set_MJDay(quint16 val_MJD);
	bool setDay(quint8 day);
	bool setMonth(quint8 month);
	void setYear(quint16 year);

	//arithmetic
	AstroDate addDays(quint16 days) const;
	AstroDate subtractDays(quint16 days) const;

	void calculateGregorianDate(void);
	void calculateModifiedJulianDay(void);

private:
    void calcISOWeekNr(void);

private:
	quint8 day;
	quint8 week;
	quint8 month;
	quint16 year;
	quint16 mod_julian_day; // contains the Modified Julian Day number (only full days, at 0:00 hours)
};

inline AstroDate const operator+(AstroDate const &l_hand, AstroDate const &r_hand)
{
	return AstroDate(l_hand) += r_hand;
}

inline AstroDate const operator-(AstroDate const &l_hand, AstroDate const &r_hand)
{
	return AstroDate(l_hand) -= r_hand;
}

inline QDataStream& operator<< (QDataStream &out, const AstroDate &date) {
	out << (quint8) date.day << (quint8) date.month << (quint16) date.year << (quint16) date.mod_julian_day;
	return out;
}

inline QDataStream& operator>> (QDataStream &in, AstroDate &date) {
	in >> date.day >> date.month >> date.year >> date.mod_julian_day;
	return in;
}

#endif /* ASTRODATE_H_ */
