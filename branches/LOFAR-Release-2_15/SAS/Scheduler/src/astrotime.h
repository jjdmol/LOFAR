/*
 * astrotime.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 27-jan-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/astrotime.h $
 *
 */

#ifndef ASTROTIME_H_
#define ASTROTIME_H_

#include <string>
#include <fstream>
#include <QDataStream>
#include <QTime>

#define SMALLEST_DETECTABLE_TIME_DIF 1.15e-5 // almost equal to one julian second

class AstroDateTime;

class AstroTime
{
public:
	AstroTime();
	AstroTime(const double &time);
	AstroTime(const AstroDateTime &);
	AstroTime(const QTime &);
	AstroTime(const char *chr);
	AstroTime(const std::string &);
	AstroTime(const QString &);
	AstroTime(qint32 hours, qint8 minutes, qint8 seconds);
	virtual ~AstroTime();

	// overloaded operators
	friend QDataStream& operator<< (QDataStream &out, const AstroTime &time); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, AstroTime &time); // used for reading binary data from file

	AstroTime &operator=(const AstroDateTime &);
	AstroTime &operator=(const QString &);
	AstroTime &operator+=(const AstroTime &right);
	AstroTime &operator-=(const AstroTime &right);
	bool operator==(const AstroTime &right) const;
	bool operator!=(const AstroTime & right) const;
	bool operator>(const AstroTime &right) const;
	bool operator<(const AstroTime &right) const;
	bool operator>=(const AstroTime &right) const;
	bool operator<=(const AstroTime &right) const;

	AstroTime addHours(unsigned int hrs) const;
	AstroTime subtractHours(unsigned int hrs) const;
	AstroTime addMinutes(unsigned int mins) const;
	AstroTime subtractMinutes(unsigned int mins) const;
	AstroTime addSeconds(unsigned int seconds) const;
	AstroTime subtractSeconds(unsigned int seconds) const;

	QTime toQTime(void) const {return QTime(hours, minutes, seconds);}

	void calculateJulianTime(void); // updates the julian attribute to correspond to the time
	void calculateHMS(void); // updates the hours, minutes and seconds to reflect the julian time
    void clearTime(void) {hours = 0; minutes = 0; seconds = 0; julian_time = 0.0;}

	// Get methods
    int getHours() const {return hours;}
    short int getMinutes() const {return minutes;}
    short int getSeconds() const {return seconds;}
    int totalSeconds() const {return /*carryDays * 86400 + */hours * 3600 + minutes * 60 + seconds;}
    double toJulian(void) const {return julian_time;}
    double getJulianTime(void) const {return julian_time;}
    const AstroTime &getTime(void) const {return *this;}
    std::string toString(short mode = 0) const;
    bool isSet(void) const { if ((hours == 0) && (minutes == 0) && (seconds == 0) /*& (carryDays == 0)*/) return false; else return true; }

    // Set methods
	bool setTimeStr(const QString &time);
	bool setTimeStr(const std::string &time) {return setTimeStr(QString(time.c_str()));}
	void setHours(qint16 hours) { this->hours = hours; }
	bool setMinutes(qint8 minutes);
	bool setSeconds(qint8 seconds);
	void setJulianTime(double jt);

private:
	qint32 hours; // hours (counts beyond 24)
	qint8 minutes; // minutes (Solar Time)
	qint8 seconds; // seconds (Solar Time)
	double julian_time; // julian date
};

inline const AstroTime operator-(const AstroTime &l_hand, const AstroTime &r_hand) {
	return AstroTime(l_hand) -= r_hand;
}

inline const AstroTime operator+(const AstroTime &l_hand, const AstroTime &r_hand) {
	return AstroTime(l_hand) += r_hand;
}

inline QDataStream& operator<< (QDataStream &out, const AstroTime &time) {
	out << (qint32) time.hours
	    << (qint8) time.minutes
	    << (qint8) time.seconds
	    << time.julian_time;
	return out;
}

inline QDataStream& operator>> (QDataStream &in, AstroTime &time) {
	in >> time.hours >> time.minutes >> time.seconds >> time.julian_time;
	return in;
}

#endif /* ASTROTIME_H_ */
