/*
 * astrodate.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Feb 13, 2009
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/astrodate.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "astrodate.h"
#include <string>
#include <stdlib.h>
#include <cmath>
using std::string;
#ifdef DEBUG_SCHEDULER
#include <iostream>
#endif

const char * DAY_NAMES_ENG[7] = {"Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday", "Sunday"};
const char * DAY_SHORT_NAMES_ENG[7] = {"Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
const char * MONTH_SHORT_NAMES_ENG[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
const char * MONTH_NAMES_ENG[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

AstroDate::AstroDate() :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	//mod_julian_day = JUL_GREG_CALENDAR_EPOCH;
}
/*
AstroDate::AstroDate(const AstroDate &other)  :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	this->day = other.day;
	this->month = other.month;
	this->year = other.year;
	this->week = other.week;
	this->mod_julian_day = other.mod_julian_day;
}
*/

AstroDate::AstroDate(const QDate &date) {
	day = date.day();
	month = date.month();
	year = date.year();
//	week = date.weekNumber();
	calculateModifiedJulianDay(); // also calculates ISO week number
}


AstroDate::AstroDate(const char *chr) :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	setDateStr(QString(chr));
}

AstroDate::AstroDate(const QString &dateStr) :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	setDateStr(dateStr);
}

AstroDate::AstroDate(const std::string &dateStr) :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	setDateStr(dateStr);
}

AstroDate::AstroDate(unsigned int julian_day) :
	day(0), week(0), month(0), year(0), mod_julian_day(0)
{
	set_MJDay(julian_day);
//	calculateGregorianDate();
}

AstroDate::AstroDate(quint8 d, quint8 mnth, quint16 yr) :
	day(d), week(0), month(mnth), year(yr), mod_julian_day(0)
{
	calculateModifiedJulianDay();
}

AstroDate::~AstroDate() {
}

/*
ostream& operator<< (ostream &out, const AstroDate &date) {
	out << day << month << year << mod_julian_day;
	return out;
}
*/

AstroDate & AstroDate::operator=(const QString &date) {
	year = 0;
	month = 0;
	day = 0;
	week = 0;
	mod_julian_day = 0;
	setDateStr(date);
	return *this;
}

AstroDate & AstroDate::operator=(int modJulian) {
	mod_julian_day = modJulian;
	calculateGregorianDate();
	calcISOWeekNr();
	return *this;
}

AstroDate &AstroDate::operator++() {
	++mod_julian_day;
	calculateGregorianDate();
	calcISOWeekNr();
	return *this;
}

const AstroDate AstroDate::operator++(int) {
	AstroDate copy(*this);
	++(*this);
	return copy;
}

AstroDate &AstroDate::operator--() {
	--mod_julian_day;
	calculateGregorianDate();
	calcISOWeekNr();
	return *this;
}

const AstroDate AstroDate::operator--(int) {
	AstroDate copy(*this);
	--(*this);
	return copy;
}


AstroDate &AstroDate::operator+=(AstroDate const &other)
{
	this->mod_julian_day += other.toJulian();
	calculateGregorianDate();
	calcISOWeekNr();
	return *this;
}

AstroDate &AstroDate::operator-=(AstroDate const &other)
{
	this->mod_julian_day -= other.toJulian();
	calculateGregorianDate();
	calcISOWeekNr();
	return *this;
}

bool AstroDate::operator==(const AstroDate &right) const {
	if ((this->year == right.year) && (this->month == right.month) && (this->day == right.day))
		return true;
	else return false;
}

bool AstroDate::operator==(int right) const {
	if (this->mod_julian_day == right)
		return true;
	else return false;
}

bool AstroDate::operator!=(const AstroDate & right) const {
	return !(*this == right);
}

bool AstroDate::operator>(const AstroDate &right) const {
	if (this->year > right.year) return true;
	else if ((this->year == right.year) && (this->month > right.month)) return true;
	else if ((this->year == right.year) && (this->month == right.month)  && (this->day > right.day)) return true;
	else return false;
}

bool AstroDate::operator<(const AstroDate &right) const {
	if (this->year < right.year) return true;
	else if ((this->year == right.year) && (this->month < right.month)) return true;
	else if ((this->year == right.year) && (this->month == right.month)  && (this->day < right.day)) return true;
	else return false;
}

bool AstroDate::operator>=(const AstroDate &right) const {
	if ( (*this > right) || (*this == right) ) return true;
	else return false;
}

bool AstroDate::operator<=(const AstroDate &right) const {
	if ( (*this < right) || (*this == right) )return true;
	else return false;
}


bool AstroDate::setDay(quint8 day) {
// sets the day value. Days can range from 0 to 31
	if (day > 0 && day <= 31) {
		this->day = day;
        calculateModifiedJulianDay();
		return true;
	}
	else {
		return false;
	}
}


bool AstroDate::setMonth(quint8 month) {
// sets the month value. Month can range from 0 to 12
	if (month > 0 && month <= 12) {
		this->month = month;
        calculateModifiedJulianDay();
		return true;
	}
	else {
		return false;
	}
}

void AstroDate::setYear(quint16 year) {
	this->year = year;
    calculateModifiedJulianDay();
}

void AstroDate::setDateStr(const QString &dateStr) {
	QDate date(QDate::fromString(dateStr, "yyyy-MM-dd"));
	year = date.year();
	month = date.month();
	day = date.day();
	calculateModifiedJulianDay();
}

void AstroDate::set_MJDay(quint16 val_MJD) {
	mod_julian_day = val_MJD;
	calculateGregorianDate();
	calcISOWeekNr();
}

AstroDate AstroDate::addDays(quint16 days) const {
	AstroDate result(*this);
	result.set_MJDay(result.toJulian() + days);
	result.calculateGregorianDate();
	result.calcISOWeekNr();
	return result;
}

AstroDate AstroDate::subtractDays(quint16 days) const {
	AstroDate result(*this);
	result.set_MJDay(result.toJulian() - days);
	result.calculateGregorianDate();
	result.calcISOWeekNr();
	return result;
}

void AstroDate::calculateGregorianDate(void) {
	// Updates the Gregorian data from the Modified Julian Date
	// taken from http://www.astro.uu.nl/~strous/AA/en/reken/juliaansedag.html
	double J = mod_julian_day + J2000_EPOCH;
	double p = floor(J + 0.5); // eq. 3
	double s1 = p + 68569 ; // eq. 4
	double n = floor(4 * s1 / 146097); // eq. 5
	double s2 = s1 - floor((146097 * n + 3) / 4); // eq. 6
	double i = floor(4000 * (s2 + 1) / 1461001); // eq. 7
	double s3 = s2 - floor(1461 * i / 4) + 31; // eq. 8
	double q = floor(80 * s3 / 2447); // eq 9
	double e = s3 - floor(2447*q/80); // eq. 10
	double s4 = floor(q/11); // eq. 11
	day = static_cast<unsigned short int>(e + J - p + 0.5);
	month = static_cast<unsigned short int>(q + 2 - 12 * s4);
//	year = static_cast<unsigned int>(100 * (n - 49) + i + s4 + 4713 + 1858) ;
	year = static_cast<unsigned int>(100 * (n - 49) + i + s4);
//	year = 100 * (n - 49) + i + s4;
}

void AstroDate::calculateModifiedJulianDay(void) {
	// updates the Modified Julian data from the Gregorian date
	// taken from : http://www.astro.uu.nl/~strous/AA/en/reken/juliaansedag.html
	if (day!=0 && month!=0 && year!=0) {
		double m = month, j = year, d = day;
		if (m < 3) { m += 12; --j; } // step 1

		  //A = Y/100
		  //B = A/4
		  //C = 2-A+B

		  //double E = floor(365.25 * (j+4716));
		  //double F = floor(30.6001 * (M+1));
		  mod_julian_day = static_cast<int>(2 - floor(j/100) + floor(j/400) + d + floor(365.25 * (j+4716)) + floor(30.6001 * (m + 1)) - 1524 - J2000_EPOCH);

		  calcISOWeekNr();

		//double c = 2 - floor(j/100) + floor(j/400); // step 2
		//mod_julian_day = static_cast<unsigned int>(floor(floor(1461 * (j + 4716) / 4) + floor(153 * (m + 1) / 5) + d + c - 1524.5)); // step 3, Julian Day number
	}
}

unsigned int AstroDate::getDayOfTheWeek(void) const {
	return (mod_julian_day + static_cast<int>(J2000_EPOCH)) % 7;
}

const char *AstroDate::getDayString(void) const {
	return DAY_NAMES_ENG[getDayOfTheWeek()];
}

bool AstroDate::isLeapYear(unsigned int year) const {
	if (year % 400 == 0) return true;
	else if (year % 100 == 0) return false;
	else if (year % 4 == 0) return true;
	else return false;
}

void AstroDate::calcISOWeekNr(void) {
	int J = mod_julian_day + static_cast<int>(J2000_EPOCH);
	int d4 = (J+31741 - (J % 7)) % 146097 % 36524 % 1461;
	int L = d4 / 1460;
	int d1 = ((d4-L) % 365) + L;
	week = d1/7 + 1;
}

std::string AstroDate::toString(short mode) const {
	char str[25];
	switch (mode) {
	case 1:
		sprintf(str, "%4.4u-%2.2u-%2.2u", getYear(), getMonth(), getDay()); // appropriate for sorting
		break;
	case 0:
	default:
		sprintf(str, "%4.4u-%2.2u-%2.2u", getYear(), getMonth(), getDay());
		break;
	}
	std::string strdate(str);
	return strdate;
}
