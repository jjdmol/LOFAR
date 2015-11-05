/*
 * DigitalBeam.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Dec 6, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DigitalBeam.h $
 *
 */

#ifndef DIGITALBEAM_H_
#define DIGITALBEAM_H_

#include "lofar_scheduler.h"
#include "lofar_utils.h"
#include "Angle.h"
#include "astrotime.h"
#include "TiedArrayBeam.h"
#include <string>
#include <vector>

enum beamDirectionType {
	_BEGIN_DIRECTION_TYPES,
	DIR_TYPE_J2000 = _BEGIN_DIRECTION_TYPES, // Right ascension & declination
	DIR_TYPE_B1950,
	DIR_TYPE_ITRF,
	DIR_TYPE_HADEC,// hour angle + declination
	DIR_TYPE_AZELGEO,  // Azimuth & Elevation
	DIR_TYPE_TOPO,
	DIR_TYPE_ICRS,
	DIR_TYPE_APP,
	DIR_TYPE_GALACTIC,
	DIR_TYPE_ECLIPTIC,
	DIR_TYPE_COMET,
	DIR_TYPE_MERCURY,
	DIR_TYPE_VENUS,
	DIR_TYPE_MARS,
	DIR_TYPE_JUPITER,
	DIR_TYPE_SATURN,
	DIR_TYPE_URANUS,
	DIR_TYPE_NEPTUNE,
	DIR_TYPE_PLUTO,
	DIR_TYPE_SUN,
	DIR_TYPE_MOON,
	DIR_TYPE_UNDEFINED,
	_END_DIRECTION_TYPES
};

extern const char * BEAM_DIRECTION_TYPES[_END_DIRECTION_TYPES];

extern beamDirectionType stringToBeamDirectionType(const std::string &str);

class DigitalBeam {
public:
	DigitalBeam() :
		itsDirectionType(DIR_TYPE_J2000), itsUnits(ANGLE_PAIRS_HMS_DMS), itsTabRingSize(0), itsNrTabRings(0), itsSubbandNotationChange(false) { }
	friend QDataStream& operator<< (QDataStream &out, const DigitalBeam &digiBeam); // used for writing data to binary file
	friend QDataStream& operator>> (QDataStream &in, DigitalBeam &digiBeam); // used for reading data from binary file

	bool operator!=(const DigitalBeam & right) const;
	bool operator==(const DigitalBeam & right) const {return (!(*this != right));}

	const std::string &target(void) const {return itsTarget;}
	const Angle &angle1(void) const {return itsAngle1;}
	const Angle &angle2(void) const {return itsAngle2;}
	beamDirectionType directionType(void) const {return itsDirectionType;}
	anglePairs units(void) const {return itsUnits;}
	const AstroTime &duration(void) const {return itsDuration;}
	const AstroTime &startTime(void) const {return itsStartTime;}
	const std::vector<unsigned> &subbandList(void) const {return itsSubbandList;}
	QString subbandsStr(void) const {return Vector2StringList(itsSubbandList);}
	unsigned nrSubbands(void) const {return itsSubbandList.size();}
	unsigned nrManualTABs(void) const {return itsTiedArrayBeams.size();}
	unsigned nrIncoherentTABs(void) const;
	unsigned nrCoherentTABs(void) const;
	unsigned nrTABrings(void) const {return itsNrTabRings;}
	unsigned nrRingTABs(void) const {return (3 * itsNrTabRings * (itsNrTabRings + 1) + 1);}
	const double &tabRingSize(void) const {return itsTabRingSize;}
	int nrTabRings(void) const {return itsNrTabRings;}
	const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(void) const {return itsTiedArrayBeams;}
	std::map<unsigned, TiedArrayBeam> &getTiedArrayBeamsForChange(void) {return itsTiedArrayBeams;}
	bool subbandNotationChange(void) const {return itsSubbandNotationChange; }

	void setTarget(const std::string &target) {itsTarget = target;}
	void setAngle1(const Angle &angle) {itsAngle1 = angle;}
	void setAngle1Radian(const double &rad) {itsAngle1.setRadianAngle(rad);}
	void setAngle1HMS(const std::string &hms) {itsAngle1.setHMSangleStr(hms);}
	void setAngle1DMS(const std::string &dms) {itsAngle1.setDMSangleStr(dms);}
	void setAngle1Degree(const double &degrees) {itsAngle1.setDegreeAngle(degrees);}
	void setAngle2(const Angle &angle) {itsAngle2 = angle;}
	void setAngle2Radian(const double &rad) {itsAngle2.setRadianAngle(rad);}
	void setAngle2HMS(const std::string &hms) {itsAngle2.setHMSangleStr(hms);}
	void setAngle2DMS(const std::string &dms) {itsAngle2.setDMSangleStr(dms);}
	void setAngle2Degree(const double &degrees) {itsAngle2.setDegreeAngle(degrees);}
	void setDirectionType(beamDirectionType directionType) {itsDirectionType = directionType;}
	void setUnits(anglePairs units) {itsUnits = units;}
    void clearDuration(void) {itsDuration.clearTime();}
	void setDuration(const AstroTime &duration) {itsDuration = duration;}
	void setDuration(unsigned seconds) {itsDuration = itsDuration.addSeconds(seconds);}
	void setTimeStr(const QString &timeStr) {itsDuration.setTimeStr(timeStr);}
    void zeroStartTime(void) {itsStartTime.clearTime();}
	void setStartTime(const AstroTime &startTime) {itsStartTime = startTime;}
	void setStartTime(unsigned seconds) {itsStartTime = itsStartTime.addSeconds(seconds);}
	void setSubbandList(const std::vector<unsigned> &subbandList) {itsSubbandList = subbandList;}
	bool setSubbandList(const QString &subbands);
	void clearSubbandList(void) {itsSubbandList.clear();}
	bool addSubband(quint16 subband);
	void setTiedArrayBeams(const std::map<unsigned, TiedArrayBeam> &tbeams) {itsTiedArrayBeams = tbeams;}
	bool addTiedArrayBeam(unsigned beamNr, const TiedArrayBeam &tiedArrayBeam) {
		return itsTiedArrayBeams.insert(std::map<unsigned, TiedArrayBeam>::value_type(beamNr, tiedArrayBeam)).second;
	}
	void setTabRingSize(const double &tabRingSize) {itsTabRingSize = tabRingSize;}
	void setNrTabRings(int nrTabRings) {itsNrTabRings = nrTabRings;}
	void clear(void);
	void setSubbandNotationChange(bool change) {itsSubbandNotationChange = change;}

	virtual ~DigitalBeam();

private:
	std::string itsTarget;
	Angle itsAngle1, itsAngle2;
	beamDirectionType itsDirectionType;
	anglePairs itsUnits;
	AstroTime itsDuration, itsStartTime;
	double itsTabRingSize; // itsTabRingSize and itsNrTabRings fold out in COHERENT TABs
	unsigned itsNrTabRings;
	std::vector<unsigned> itsSubbandList;
	bool itsSubbandNotationChange; // to keep track if changes were made to the subband list string (e.g. sequence change)
	std::map<unsigned, TiedArrayBeam> itsTiedArrayBeams;
};

#endif /* DIGITALBEAM_H_ */
