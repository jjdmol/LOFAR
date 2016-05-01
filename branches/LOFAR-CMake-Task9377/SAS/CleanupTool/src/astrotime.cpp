/*
 * astrotime.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision: 11420 $
 * Last change by : $Author: jong $
 * Change date	  : $Date: 2014-01-06 11:04:58 +0000 (Mon, 06 Jan 2014) $
 * First creation : 27-jan-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Cleanup/astrotime.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "lofar_utils.h"
#include "astrotime.h"
#include "astrodatetime.h"
#include "Controller.h"
#include <QTime>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <cmath>
using std::string;
using std::cout;
using std::endl;

AstroTime::AstroTime() :
	hours(0), minutes(0), seconds(0), julian_time(0.0)
{
}

AstroTime::AstroTime(const double &time) :
	julian_time(time)
{
	calculateHMS();
}

// needed to be able to subtract two AstroDateTimes and get a duration as AstroTime (hours possible greater than 24)
AstroTime::AstroTime(const AstroDateTime &other) {
	hours = other.getHours();
	minutes= other.getMinutes();
	seconds = other.getSeconds();
	julian_time = other.getJulianTime();
}

AstroTime::AstroTime(const QTime &time) {
	hours = time.hour();
	minutes = time.minute();
	seconds = time.second();
	calculateJulianTime();
}

AstroTime::AstroTime(const char *time) :
	hours(0), minutes(0), seconds(0), julian_time(0.0)
{
	setTimeStr(QString(time));
}

AstroTime::AstroTime(const std::string &time) :
	hours(0), minutes(0), seconds(0), julian_time(0.0)
{
	setTimeStr(time);
}


AstroTime::AstroTime(const QString &time) :
	hours(0), minutes(0), seconds(0), julian_time(0.0)
{
	setTimeStr(time);
}

AstroTime::AstroTime(qint32 hour, qint8 min, qint8 sec) :
	hours(hour), minutes(min), seconds(sec), julian_time(0.0)
{
	calculateJulianTime();
}

AstroTime::~AstroTime()
{

}

AstroTime &AstroTime::operator=(const QString &time) {
	seconds = 0;
	minutes = 0;
	hours = 0;
	julian_time = 0;
	setTimeStr(time);
	return *this;
}

AstroTime &AstroTime::operator=(const AstroDateTime &other) {
	julian_time = other.toJulian();
	calculateHMS();
	return *this;
}

AstroTime &AstroTime::operator+=(const AstroTime &right) {
	this->julian_time += right.toJulian();
	calculateHMS();
	return *this;
}

AstroTime &AstroTime::operator-=(AstroTime const &right) {
	this->julian_time -= right.toJulian();
	calculateHMS();
	return *this;
}

bool AstroTime::operator==(const AstroTime &right) const {
	return (fabs(julian_time - right.getJulianTime()) < SMALLEST_DETECTABLE_TIME_DIF);
}

bool AstroTime::operator!=(const AstroTime & right) const {
	return !(*this == right);
}

bool AstroTime::operator>(const AstroTime &right) const {
	return (julian_time - right.getJulianTime()) > SMALLEST_DETECTABLE_TIME_DIF;
}

bool AstroTime::operator<(const AstroTime &right) const {
	return (julian_time - right.getJulianTime()) < -SMALLEST_DETECTABLE_TIME_DIF;
}

bool AstroTime::operator>=(const AstroTime &right) const {
	if ( (*this == right) || (*this > right) ) return true;
	else return false;
}

bool AstroTime::operator<=(const AstroTime &right) const {
	if ( (*this == right) || (*this < right) ) return true;
	else return false;
}

AstroTime AstroTime::addHours(unsigned int hrs) const {
	AstroTime result(*this);
//	hrs %= 24;
	result.setHours(result.getHours() + hrs);
//	if (result.getHours() >= 24) {
//		result.setHours(result.getHours() - 24);
//	}
	result.calculateJulianTime();
	return result;
}

AstroTime AstroTime::subtractHours(unsigned int hrs) const {
	AstroTime result(*this);
//	hrs %= 24;
	result.setHours(result.getHours() - hrs);
//	if (result.getHours() < 0) {
//		result.setHours(result.getHours() + 24);
//	}
	result.calculateJulianTime();
	return result;
}

AstroTime AstroTime::addMinutes(unsigned int mins) const {
	AstroTime result(*this);
	int hrs = mins / 60;
	if (hrs >= 1) {
		result = result.addHours(hrs);
		mins -= hrs * 60;
	}
	int temp = result.getMinutes() + mins;
	if (temp >= 60) {
		result.setMinutes(temp - 60);
		result = result.addHours(1);
	}
	else {
		result.setMinutes(temp);
	}
	result.calculateJulianTime();
	return result;
}

AstroTime AstroTime::subtractMinutes(unsigned int mins) const {
	AstroTime result(*this);
	int hrs = mins / 60;
	if (hrs >= 1) {
		result = result.subtractHours(hrs);
		mins -= hrs * 60;
	}
	int temp = result.getMinutes() - mins;
	if (temp < 0) {
		result.setMinutes(temp + 60);
		result = result.subtractHours(1);
	}
	else {
		result.setMinutes(temp);
	}
	result.calculateJulianTime();
	return result;
}

AstroTime AstroTime::addSeconds(unsigned int seconds) const {
	AstroTime result(*this);
	int hrs = seconds / 3600;
	if (hrs >= 1) {
		result = result.addHours(hrs);
		seconds -= hrs * 3600;
	}
	int mins = seconds / 60;
	if (mins >= 1) {
		result = result.addMinutes(mins);
		seconds -= mins * 60;
	}
	int temp = result.getSeconds() + seconds;
	if (temp >= 60) {
		result.setSeconds(temp - 60);
		result = result.addMinutes(1);
	}
	else {
		result.setSeconds(temp);
	}
	result.calculateJulianTime();
	return result;
}

AstroTime AstroTime::subtractSeconds(unsigned int seconds) const {
	AstroTime result(*this);
	int hrs = seconds / 3600;
	if (hrs >= 1) {
		result = result.subtractHours(hrs);
		seconds -= hrs * 3600;
	}
	int mins = seconds / 60;
	if (mins >= 1) {
		result = result.subtractMinutes(mins);
		seconds -= mins * 60;
	}
	int temp = result.getSeconds() - seconds;
	if (temp < 0) {
		result.setSeconds(temp + 60);
		result = result.subtractMinutes(1);
	}
	else {
		result.setSeconds(temp);
	}
	result.calculateJulianTime();
	return result;
}

void  AstroTime::setJulianTime(double jt) {
	julian_time = jt;// - floor(jt);
	calculateHMS();
}

void  AstroTime::calculateHMS(void) {
	double h = julian_time * 24.0; // add 12 hours because J2000 EPOCH has its zero at noon
	hours = static_cast<int>(floor(h));

	double m = (h - hours) * 60.0;
	minutes = static_cast<int>(m);

	seconds = static_cast<int>((m - minutes)*60.0 + 0.5);
	if (seconds >= 60) {
		minutes += 1;
		seconds -= 60;
	}
	if (minutes >= 60) {
		++hours;
		minutes -= 60;
	}
	while (hours < 0) { hours += 24;}
}

bool AstroTime::setTimeStr(const QString &timeStr) {
	if (!timeStr.isEmpty()) {
		QString t(timeStr.trimmed());
		int pos = t.indexOf(':');
		int pos1 = t.indexOf(':',pos+1);
		hours = t.left(pos).toInt();
		minutes = t.mid(pos+1, pos1 - pos -1).toInt();
		seconds = t.mid(pos1+1).toInt();
		calculateJulianTime();
		return true;
	}
	else return false;
}

void AstroTime::calculateJulianTime(void) {
	julian_time = static_cast<double>(seconds) / 86400 +
				  static_cast<double>(minutes) / 1440 +
				  static_cast<double>(hours) / 24;// +
				//  static_cast<double>(carryDays);
}

bool AstroTime::setMinutes(qint8 minutes) {
// sets the minutes value. Minutes can range from 0 to 59
	if (minutes <= 59) {
		this->minutes = minutes;
		return true;
	}
	else return false;
}

bool AstroTime::setSeconds(qint8 seconds) {
	// sets the seconds value. Seconds can range from 0 to 59
	if (seconds <= 59) {
		this->seconds = seconds;
		return true;
	}
	else return false;
}

std::string AstroTime::toString(short mode) const {
	char str[15];
	switch(mode) {
	case 1:
		sprintf(str, "%2.2i:%2.2i:%2.2i", hours, minutes, seconds); // always two digits for hours
		break;
	case 3:
		sprintf(str, "%i:%2.2i:%2.2i", hours, minutes, seconds); // display hours without lead zeros
		break;
	default:
		sprintf(str, "%4.4i:%2.2i:%2.2i", hours, minutes, seconds); // display 4 digit hours
		break;
	}
	std::string strtime(str);
	return strtime;
}
