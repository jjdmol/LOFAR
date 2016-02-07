/*
 * astrodatetime.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 27-jan-2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/astrodatetime.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "astrodatetime.h"
#include <QDateTime>
#include <string>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <cmath>
#include <cstdio>
using std::string;
using std::cerr;
using std::cout;
using std::endl;

AstroDateTime::AstroDateTime()
{
}

AstroDateTime::AstroDateTime(const std::string &date_time)
{
	setDateTime(date_time);
}

AstroDateTime::AstroDateTime(const QString &date_time)
{
	setDateTime(date_time);
}

AstroDateTime::AstroDateTime(double mod_julian)
{
	set_MJDay(static_cast<unsigned>(mod_julian));
	setJulianTime(mod_julian - floor(mod_julian));
}

AstroDateTime::AstroDateTime(int julian_day, double &time)
{
	set_MJDay(julian_day);
	setJulianTime(time);
}

AstroDateTime::AstroDateTime(int julian_day)
{
	this->set_MJDay(julian_day);
}

AstroDateTime::AstroDateTime(const AstroDate &date, const AstroTime &time)
{
	this->setDay(date.getDay());
	this->setMonth(date.getMonth());
	this->setYear(date.getYear());
	this->setSeconds(time.getSeconds());
	this->setMinutes(time.getMinutes());
	this->setHours(time.getHours());
	calculateModifiedJulianDate();
}

AstroDateTime::AstroDateTime(const QDateTime &date) {
	this->setDay(date.date().day());
	this->setMonth(date.date().month());
	this->setYear(date.date().year());
	this->setSeconds(date.time().second());
	this->setMinutes(date.time().minute());
	this->setHours(date.time().hour());
	calculateModifiedJulianDate();
}

AstroDateTime::AstroDateTime(AstroTime const &time)
{
	this->setJulianTime(time.getJulianTime());
}

AstroDateTime::AstroDateTime(unsigned short day, unsigned short month, unsigned int year, int hours, short int minutes, short int seconds) {
	this->setDay(day);
	this->setMonth(month);
	this->setYear(year);
	this->setSeconds(seconds);
	this->setMinutes(minutes);
	this->setHours(hours);
	calculateModifiedJulianDate();
}

AstroDateTime::~AstroDateTime()
{

}

QDataStream& operator<< (QDataStream &out, const AstroDateTime &date_time) {
	out << date_time.getDate() << date_time.getTime();
	return out;
}

QDataStream& operator>> (QDataStream &in, AstroDateTime &date_time) {
	AstroDate date;
	AstroTime time;
	in >> date >> time;
	date_time.setDate(date);
	date_time.setTime(time);
	return in;
}

AstroDateTime AstroDateTime::addDays(unsigned int days) const {
	AstroDateTime result(*this);
	result.set_MJDay(static_cast<unsigned>(result.toJulian()) + days);
	return result;
}

AstroDateTime AstroDateTime::subtractDays(unsigned int days) const {
	AstroDateTime result(*this);
	result.set_MJDay(static_cast<unsigned>(result.toJulian()) - days);
	return result;
}

AstroDateTime AstroDateTime::addHours(unsigned int hours) const {
	AstroDateTime result(*this);
	unsigned days = hours / 24;
	if (days >= 1) {
		result = result.addDays(days);
		hours %= (days * 24);
	}

	result.setTime(static_cast<AstroTime>(result).addHours(hours));
	if (result.getHours() >= 24) {
		result.setDay(result.getDay() + 1);
		result.setHours(result.getHours() - 24);
	}
	return result;
}

AstroDateTime AstroDateTime::subtractHours(unsigned int hours) const {
	AstroDateTime result(*this);
	unsigned days = hours / 24;
	if (days >= 1) {
		result = result.subtractDays(days);
		hours %= (days * 24);
	}

	result.setTime(static_cast<AstroTime>(result).subtractHours(hours));
	return result;
}


AstroDateTime AstroDateTime::addMinutes(unsigned int mins) const {
	AstroDateTime result(*this);
	unsigned hours = mins / 60;
	if (hours >= 1) {
		result = result.addHours(hours);
		mins %= (hours * 60);
	}

	result.setTime(static_cast<AstroTime>(result).addMinutes(mins));
	return result;
}

AstroDateTime AstroDateTime::addSeconds(unsigned int seconds) const {
	AstroDateTime result(*this);
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

AstroDateTime AstroDateTime::subtractMinutes(unsigned mins) const {
	AstroDateTime result(*this);
	unsigned hours = mins / 60;
	if (hours >= 1) {
		result = result.subtractHours(hours);
		mins %= (hours * 60);
	}

	result.setTime(static_cast<AstroTime>(result).subtractMinutes(mins));
	return result;
}

AstroDateTime AstroDateTime::subtractSeconds(unsigned seconds) const {
	AstroDateTime result(*this);
	unsigned hours = seconds / 3600;
	if (hours >= 1) {
		result = result.subtractHours(hours);
		seconds %= (hours * 3600);
	}

	result.setTime(static_cast<AstroTime>(result).subtractSeconds(seconds));
	return result;
}

AstroDateTime AstroDateTime::addWeeks(unsigned int weeks) const {
	AstroDateTime result(*this);
	result = result.addDays(weeks*7);
	return result;
}

AstroDateTime AstroDateTime::subtractWeeks(unsigned int weeks) const {
	AstroDateTime result(*this);
	result = result.subtractDays(weeks*7);
	return result;
}

void AstroDateTime::calculateGregorianDateTime(void) {
	calculateGregorianDate();
	calculateHMS();
}


AstroTime AstroDateTime::toLST(void) const {
	double fT = (get_MJDay() - 0.5) / 36525.0;
	double fT0 = 6.697374558 + (2400.051336 * fT) + (0.000025862 * fT * fT);

	while (fT0 > 24) {
		fT0 -= 24;
	}
	while (fT0 < 0) {
		fT0 += 24;
	}

	double fUT = (double)getHours() + (double)getMinutes() / 60.0 + (double)getSeconds() / 3600.0;

	fUT = fUT * 1.002737909;
	double fGST = fUT + fT0;
	while (fGST > 24)
	{
		fGST -= 24;
	}

	// fGST is the Greenwich Sidereal Time in decimal hours
	return AstroTime((fGST + 0.45795352) / 24.0 ); // 0.45795352 is the longitude of CS002 in decimal hours
}


AstroDateTime AstroDateTime::UTfromLST(const AstroTime &lstTime, const AstroDate &calendarDate) {

	bool bPrevDay(false), bNextDay(false);

	double fT = (calendarDate.get_MJDay() - 0.5) / 36525.0;
	double fT0 = 6.697374558 + (2400.051336 * fT) + (0.000025862 * fT * fT);

	// put in 24 hours:
	while (fT0 > 24) {
		fT0 -= 24;
	}
	while (fT0 < 0) {
		fT0 += 24;
	}

	double fGST = lstTime.getJulianTime() * 24.0 - 0.45795352; // 0.45795352 is the longitude of CS002 in decimal hours

	fGST = fGST - fT0;

	while (fGST > 24) {
		fGST -= 24;
		bNextDay = true;
	}
	while (fGST < 0) {
		fGST += 24;
		bPrevDay = true;
	}

	double fUT = fGST * 0.9972695666779660; // 1 / 1.002737909 = 0.9972695666779660


	AstroDateTime dUT(calendarDate, AstroTime(fUT / 24.0));

	if (bNextDay) {
		dUT = dUT.addDays(1);
	}
	else if (bPrevDay) {
		dUT = dUT.subtractDays(1);
	}

	return dUT;
}

void AstroDateTime::calculateModifiedJulianDate(void){
	calculateJulianTime();
	calculateModifiedJulianDay();
}

bool AstroDateTime::operator=(const QString &date_time) {
	return setDateTime(date_time);
}

bool AstroDateTime::operator=(const QDateTime &date_time) {
	setDay(date_time.date().day());
	setMonth(date_time.date().month());
	setYear(date_time.date().year());
	setHours(date_time.time().hour());
	setMinutes(date_time.time().minute());
	setSeconds(date_time.time().second());
	calculateModifiedJulianDate();
	return true;
}

bool AstroDateTime::operator=(const double &mod_julian) {
	if (mod_julian >= 0.0) {
		set_MJDay(static_cast<unsigned>(floor(mod_julian)));
		setJulianTime(mod_julian - floor(mod_julian));
		calculateGregorianDateTime();
		return true;
	}
	else return false;
}

bool AstroDateTime::operator==(const AstroDateTime &right) const {
	if ((this->getDate() == right.getDate()) && (this->getTime() == right.getTime())) { return true; }
	else return false;
}

bool AstroDateTime::operator!=(const AstroDateTime & right) const {
	if ((this->getDate() != right.getDate()) || (this->getTime() != right.getTime())) { return true; }
	else return false;
}

bool AstroDateTime::operator>(const AstroDateTime &right) const {
	if (this->getDate() > right.getDate()) { return true; }
	else if ((this->getDate() == right.getDate()) && (this->getTime() > right.getTime())) { return true; }
	else return false;
}

bool AstroDateTime::operator<(const AstroDateTime &right) const {
	if (this->getDate() < right.getDate()) { return true; }
	else if ((this->getDate() == right.getDate()) && (this->getTime() < right.getTime())) { return true; }
	else return false;
}

bool AstroDateTime::operator>=(const AstroDateTime &right) const {
	if (this->getDate() > right.getDate()) { return true; }
	else if ((this->getDate() == right.getDate()) && (this->getTime() >= right.getTime())) { return true; }
	else return false;
}

bool AstroDateTime::operator<=(const AstroDateTime &right) const {
	if (this->getDate() < right.getDate()) { return true; }
	else if ((this->getDate() == right.getDate()) && (this->getTime() <= right.getTime())) { return true; }
	else return false;
}

AstroDateTime &AstroDateTime::operator+=(const AstroDateTime &other) {
	double result = this->toJulian() + other.toJulian();
	int MJday = static_cast<int>(result);
	this->set_MJDay(MJday);
	this->setJulianTime(result - MJday);
	return *this;
}

AstroDateTime &AstroDateTime::operator-=(const AstroTime &other) {
	double dValue(this->toJulian() - other.toJulian());
	int days(static_cast<int>(dValue));
	this->set_MJDay(days);
	this->setJulianTime(dValue - days);
	return (*this);
}

AstroDateTime &AstroDateTime::operator+=(const AstroTime &other) {
	double sum(this->toJulian() + other.toJulian());
	int days(static_cast<int>(sum)); // i.e. floor of sum = days
	this->set_MJDay(days);
	this->setJulianTime(sum - days); // remainder sum - days is julian time that needs to be set
	return (*this);
}

AstroTime AstroDateTime::operator-=(const AstroDateTime &other) {
	double result = this->toJulian() - other.toJulian();
	return AstroTime(result);
}

// this function will return the absolute time difference
AstroTime AstroDateTime::timeDifference(const AstroDateTime &other) const {
	double tj(this->toJulian()), oj(other.toJulian());
	if (oj > tj) {
		return AstroTime(oj - tj);
	}
	else return AstroTime(tj - oj);
}

void AstroDateTime::setDate(const AstroDate &date) {
	this->setDay(date.getDay());
	this->setMonth(date.getMonth());
	this->setYear(date.getYear());
	this->set_MJDay(date.toJulian());
}

void AstroDateTime::setTime(const AstroTime &time) {
	this->setHours(time.getHours());
	this->setMinutes(time.getMinutes());
	this->setSeconds(time.getSeconds());
	this->setJulianTime(time.toJulian());
}

bool AstroDateTime::setDateTime(const QString &date_time) {
	if (!date_time.isEmpty()) {
		QDateTime dt(QDateTime::fromString(date_time, "yyyy-MM-dd hh:mm:ss"));
		const QDate &date(dt.date());
		const QTime &time(dt.time());
		setYear(date.year());
		setMonth(date.month());
		setDay(date.day());
		setHours(time.hour());
		setMinutes(time.minute());
		setSeconds(time.second());
		calculateModifiedJulianDay();
		calculateJulianTime();
		return true;
	}
	else {
		debugErr("s","AstroDateTime::setDateTime: no date specified!");
		return false;
	}
}

std::string AstroDateTime::toString(void) const {
	if (isSet()) {
		char str[25];
		sprintf(str, "%4.4u-%2.2u-%2.2u %2.2i:%2.2i:%2.2i", getYear(), getMonth(), getDay(), getHours(), getMinutes(), getSeconds());
		std::string strdate(str);
		return strdate;
	}
	return "";
}

std::string AstroDateTime::toSASDateTimeString(void) const {
	if (isSet()) {
		char str[25];
		sprintf(str, "%4.4u-%2.2u-%2.2u %2.2i:%2.2i:%2.2i", getYear(), getMonth(), getDay(), getHours(), getMinutes(), getSeconds());
		std::string strdate(str);
		return strdate;
	}
	return "";
}
