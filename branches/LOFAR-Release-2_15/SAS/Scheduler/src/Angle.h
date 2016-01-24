/*
 * Angle.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : 31-mrt-2010
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/Angle.h $
 *
 */

#ifndef ANGLE_H_
#define ANGLE_H_

#include <string>
#include <QDataStream>

enum angleUnits {
	ANGLE_UNITS_HMS,
	ANGLE_UNITS_DMS,
	ANGLE_UNITS_DEG,
	ANGLE_UNITS_RAD,
	END_ANGLE_UNITS
};

enum anglePairs {
	ANGLE_PAIRS_RADIANS,
	ANGLE_PAIRS_DECIMAL_DEGREES,
	ANGLE_PAIRS_HMS_DMS,
	ANGLE_PAIRS_DMS_DMS,
	END_ANGLE_PAIRS
};

extern const char * ANGLE_PAIRS[END_ANGLE_PAIRS];

class Angle {

	struct hms {
		qint8 hours;
		qint8 minutes;
		double seconds;
	};

	struct dms {
		qint16 degrees;
		qint8 minutes;
		double seconds;
	};

public:
	Angle();
	virtual ~Angle();

	bool operator==(const Angle & right) const;
	bool operator!=(const Angle & right) const;

//	friend std::ostream& operator<< (std::ostream &out, const Angle &angle); // used for writing data to binary file
//	friend std::istream& operator>> (std::istream &in, Angle &angle); // used for reading data from binary file
	friend QDataStream& operator<< (QDataStream &out, const Angle &angle); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, Angle &angle); // used for reading data from binary file

	// set this angle with a radian input value and then calculate the other angle units
	void setRadianAngle(const double &rad);
	void setDegreeAngle(const double &deg);
	bool setHMSangle(const hms & HMS);
	bool setHMSangleStr(const std::string &HMSstring);
	bool setDMSangle(const dms & dms);
	bool setDMSangleStr(const std::string &DMSstring);
	void clear(void);

	const double &radian(void) const {return itsRadianValue;}
	const double &degree(void) const {return itsDegreeValue;}
	const hms &HMS(void) const {return itsHMSvalue;}
	const dms &DMS(void) const {return itsDMSvalue;}
	const std::string &HMSstring(void) const {return itsHMSstr;}
	const std::string &DMSstring(void) const {return itsDMSstr;}

private:
	// convert a angle unit as string to another unit as string
//	std::string Angle::convertAngle(const std::string& valueStr, angleUnits oldUnits, angleUnits newUnit);
	// convert a decimal degree angle to hours minutes and seconds
//	void radianToHMS(void);
//	void radianToDegree(void);
//	void radianToDMs(void);
	void setHMSstring(void);
	void setDMSstring(void);
	void calcHMSfromDegrees(void); // helper function to calculate HMS from degree value
	void calcDegreesfromHMS(void); // helper function to calculate degrees from HMS value
	void calcDegreesfromDMS(void); // helper function to calculate degrees from DMS value
	void calcDMSfromDegrees(void); // helper function to calculate DMS from degrees value

private:
	double itsRadianValue;
	double itsDegreeValue;
	hms itsHMSvalue;
	dms itsDMSvalue;
	std::string itsHMSstr, itsDMSstr;
};

#endif /* ANGLE_H_ */


