/*
 * Angle.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 31-mrt-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Angle.cpp $
 *
 */

#include "lofar_scheduler.h"
#include "Angle.h"
#include "lofar_utils.h"
#include <cmath>
#include <sstream>
#include <algorithm>
#include <iomanip> // for std::setprecision()
#include <limits>

using std::min;
using std::max;
using std::stringstream;

const char * ANGLE_PAIRS[END_ANGLE_PAIRS] = {"radians", "dec.degrees", "(HMS,DMS)", "(DMS,DMS)"};

Angle::Angle()
: 	itsRadianValue(0.0), itsDegreeValue(0.0)
{
	itsDMSvalue.degrees = 0;
	itsDMSvalue.minutes = 0;
	itsDMSvalue.seconds = 0.0;
	itsHMSvalue.hours = 0;
	itsHMSvalue.minutes = 0;
	itsHMSvalue.seconds = 0.0;
	setHMSstring();
	setDMSstring();
}

Angle::~Angle() {
	// TODO Auto-generated destructor stub
}

bool Angle::operator==(const Angle & right) const {
	return fabs(this->itsRadianValue - right.itsRadianValue) < std::numeric_limits<double>::epsilon();
}

bool Angle::operator!=(const Angle & right) const {
	return !(*this == right);
}

/*
std::istream& operator>> (std::istream &in, Angle &angle) {
	read_primitive<double>(in, angle.itsRadianValue);
	read_primitive<double>(in, angle.itsDegreeValue);
	// hmsvalue
	read_primitive<unsigned>(in, angle.itsHMSvalue.hours);
	read_primitive<unsigned>(in, angle.itsHMSvalue.minutes);
	read_primitive<double>(in, angle.itsHMSvalue.seconds);
	// dms value
	read_primitive<int>(in, angle.itsDMSvalue.degrees);
	read_primitive<unsigned>(in, angle.itsDMSvalue.minutes);
	read_primitive<double>(in, angle.itsDMSvalue.seconds);

	read_string(in, angle.itsHMSstr);
	read_string(in, angle.itsDMSstr);

	return in;
}

std::ostream& operator<< (std::ostream &out, const Angle &angle) {
	write_primitive<double>(out, angle.itsRadianValue);
	write_primitive<double>(out, angle.itsDegreeValue);
	// hmsvalue
	write_primitive<unsigned>(out, angle.itsHMSvalue.hours);
	write_primitive<unsigned>(out, angle.itsHMSvalue.minutes);
	write_primitive<double>(out, angle.itsHMSvalue.seconds);
	// dms value
	write_primitive<int>(out, angle.itsDMSvalue.degrees);
	write_primitive<unsigned>(out, angle.itsDMSvalue.minutes);
	write_primitive<double>(out, angle.itsDMSvalue.seconds);

	write_string(out, angle.itsHMSstr);
	write_string(out, angle.itsDMSstr);

	return out;
}
*/
QDataStream& operator>> (QDataStream &in, Angle &angle) {
	in >> angle.itsRadianValue
	   >> angle.itsDegreeValue
	// hmsvalue
	   >> angle.itsHMSvalue.hours
	   >> angle.itsHMSvalue.minutes
	   >> angle.itsHMSvalue.seconds
	// dms value
	   >> angle.itsDMSvalue.degrees
	   >> angle.itsDMSvalue.minutes
	   >> angle.itsDMSvalue.seconds

	   >> angle.itsHMSstr
	   >> angle.itsDMSstr;

	return in;
}

QDataStream& operator<< (QDataStream &out, const Angle &angle) {
	out << angle.itsRadianValue
	    << angle.itsDegreeValue
	// hmsvalue
	    << angle.itsHMSvalue.hours
	    << angle.itsHMSvalue.minutes
	    << angle.itsHMSvalue.seconds
	// dms value
	    << angle.itsDMSvalue.degrees
	    << angle.itsDMSvalue.minutes
	    << angle.itsDMSvalue.seconds

	    << angle.itsHMSstr
	    << angle.itsDMSstr;

	return out;
}

bool Angle::setHMSangleStr(const std::string &HMSstring) {
	int p1, p2;
	p1 = HMSstring.find_first_of(':'); // typically = 3
	p2 = HMSstring.find_first_of(':', p1+1) + p1 - min(p1,2); //typ = 6
	int hours = string2Int(HMSstring.substr(0, p1));
	short sign = (hours < 0) ? -1 : 1;
	if (hours >= 24)
		return false;
	int minutes = sign * string2Int(HMSstring.substr(p1+1,p2-p1-1));
	if (minutes >= 60)
		return false;
	double seconds = string2Double(HMSstring.substr(p2+1));
	if (seconds > 59.9999999999999999)
		return false;

	itsHMSvalue.hours = hours;
	itsHMSvalue.minutes = minutes;
	itsHMSvalue.seconds = seconds;
	setHMSstring();
	// calculate degrees value
	calcDegreesfromHMS();
	// calculate radian value
	itsRadianValue = itsDegreeValue * GRAD2RAD;
	// calculate DMS value
	calcDMSfromDegrees();
	return true;
}

bool Angle::setDMSangleStr(const std::string &DMSstring) {
	int p1, p2;
	p1 = DMSstring.find_first_of(':');
	p2 = DMSstring.find_first_of(':', p1+1) + p1 - min(p1,3);
	int degrees = string2Int(DMSstring.substr(0, p1));
	short sign = (degrees < 0) ? -1 : 1;
	int minutes = sign * string2Int(DMSstring.substr(p1+1,p2-p1-1));
	if (minutes >= 60)
		return false;
	double seconds = sign * string2Double(DMSstring.substr(p2+1));
	if (seconds > 59.9999999999999999)
		return false;

	itsDMSvalue.degrees = degrees;
	itsDMSvalue.minutes = minutes;
	itsDMSvalue.seconds = seconds;

	setDMSstring();
	// calculate degrees value
	calcDegreesfromDMS();
	// calculate radian value
	itsRadianValue = itsDegreeValue * GRAD2RAD;
	// calculate DMS value
	calcHMSfromDegrees();
	return true;
}

void Angle::setRadianAngle(const double &rad) {
	itsRadianValue = fmod(rad, TWO_PI); // put radians in the range of [0,2Pi>
	// now calculate the other angle value units
	// decimal degrees
	itsDegreeValue = itsRadianValue * RAD2GRAD;
	// HMS
	calcHMSfromDegrees();
	// DMS
	calcDMSfromDegrees();
}

void Angle::setDegreeAngle(const double &deg) {
	itsDegreeValue = fmod(deg, 360.0); // put degrees in the range of [0,360>
	// now calculate the other angle value units
	// decimal degrees
	itsRadianValue = itsDegreeValue * GRAD2RAD;
	// HMS
	calcHMSfromDegrees();
	// DMS
	calcDMSfromDegrees();
}

bool Angle::setHMSangle(const Angle::hms & HMS) {
	if ((HMS.hours >= 24) | (HMS.minutes >= 60) | (HMS.seconds > 59.999999999999999)) {
		return false;
	}
	itsHMSvalue = HMS;
	setHMSstring();
	// decimal degrees
	calcDegreesfromHMS();
	// radians
	itsRadianValue = itsDegreeValue * GRAD2RAD;
	// DMS
	calcDMSfromDegrees();
	return true;
}

bool Angle::setDMSangle(const Angle::dms & dms) {
	if ((dms.minutes >= 60) | (dms.seconds > 59.99999999999999)) {
		return false;
	}
	itsDMSvalue = dms;
	setDMSstring();
	// decimal degrees
	calcDegreesfromDMS();
	// radians
	itsRadianValue = itsDegreeValue *GRAD2RAD;
	// HMS
	calcHMSfromDegrees();
	return true;
}

void Angle::setHMSstring(void) {
	stringstream sstr;
	sstr << std::fixed << std::setprecision(7) << (qint16)itsHMSvalue.hours << ":" << (qint16)itsHMSvalue.minutes << ":" << itsHMSvalue.seconds;
	itsHMSstr = sstr.str();
}

void Angle::setDMSstring(void) {
	stringstream sstr;
	sstr << std::fixed << std::setprecision(7) << (qint16)itsDMSvalue.degrees << ":" << (qint16)itsDMSvalue.minutes << ":" << itsDMSvalue.seconds;
	itsDMSstr = sstr.str();
}

void Angle::calcHMSfromDegrees(void) {
	double dTemp = itsDegreeValue / 15.0;
	itsHMSvalue.hours = static_cast<int>(dTemp);
	dTemp -= itsHMSvalue.hours; // remainder
	dTemp = dTemp * 60; // minutes
	itsHMSvalue.minutes = static_cast<int>(dTemp);
	dTemp -= itsHMSvalue.minutes;
	itsHMSvalue.seconds = dTemp * 60; // seconds
	setHMSstring();
}

void Angle::calcDegreesfromHMS(void) {
	itsDegreeValue = (static_cast<double>(itsHMSvalue.hours) / 24) * 360 +
					 (static_cast<double>(itsHMSvalue.minutes) / 60) * 15 +
					 (itsHMSvalue.seconds / 60) * 0.25;
}

void Angle::calcDegreesfromDMS(void) {
	itsDegreeValue = itsDMSvalue.degrees +
					 static_cast<double>(itsDMSvalue.minutes) / 60 +
					 itsDMSvalue.seconds / 3600;
}

void Angle::calcDMSfromDegrees(void) {
	itsDMSvalue.degrees = static_cast<int>(itsDegreeValue);
	double dTemp = itsDegreeValue - itsDMSvalue.degrees; // the floating point remainder degrees
    if (dTemp < 0) dTemp *= -1; // positive values
    itsDMSvalue.minutes = static_cast<int>(dTemp = dTemp * 60); // convert remainder to minutes
    dTemp -= itsDMSvalue.minutes; // subtract whole minutes
    itsDMSvalue.seconds = dTemp * 60; // convert remainder to seconds
    setDMSstring();
}

void Angle::clear(void) {
	itsRadianValue = 0.0f;
	itsDegreeValue = 0.0f;
	itsHMSvalue.hours = 0;
	itsHMSvalue.minutes = 0;
	itsHMSvalue.seconds = 0.0f;
	itsDMSvalue.degrees = 0;
	itsDMSvalue.minutes = 0;
	itsDMSvalue.seconds = 0.0f;
	setHMSstring();
	setDMSstring();
}

/*
void Angle::radianToHMS(void) {

}

void Angle::radianToDegree(void) {

}

void Angle::radianToDMs(void) {

}
*/
/* puts a large angle in the correct range 0 - 360 degrees */
/*
double Angle::range_degrees (const double &angle) {
    double temp;
    double range;

    if (angle >= 0.0 && angle < 360.0)
    	return(angle);

	temp = static_cast<int>(angle / 360);

	if ( angle < 0.0 )
	   	temp --;

    temp *= 360;
	range = angle - temp;
    return (range);
}
*/
