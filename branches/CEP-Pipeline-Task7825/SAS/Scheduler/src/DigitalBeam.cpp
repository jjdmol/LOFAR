/*
 * DigitalBeam.cpp
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Dec 6, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/DigitalBeam.cpp $
 *
 */

#include "DigitalBeam.h"
#include <algorithm>
#include <math.h>

const char * BEAM_DIRECTION_TYPES[_END_DIRECTION_TYPES] = {"J2000", "B1950", "ITRF", "HADEC","AZELGEO", "TOPO", "ICRS", "APP","GALACTIC",
		"ECLIPTIC", "COMET", "MERCURY", "VENUS", "MARS", "JUPITER", "SATURN", "URANUS", "NEPTUNE", "PLUTO", "SUN", "MOON", "UNDEFINED"};

beamDirectionType stringToBeamDirectionType(const std::string &str) {
	if (!str.empty()) {
		for (short i = 0; i < DIR_TYPE_UNDEFINED; ++i) {
			if (str.compare(BEAM_DIRECTION_TYPES[i]) == 0) {
				return static_cast<beamDirectionType>(i);
			}
		}
	}
	return static_cast<beamDirectionType>(DIR_TYPE_UNDEFINED);
}

DigitalBeam::~DigitalBeam() {
	// TODO Auto-generated destructor stub
}

bool DigitalBeam::addSubband(quint16 subband) {
	if (std::find(itsSubbandList.begin(), itsSubbandList.end(), subband) == itsSubbandList.end()) {
		itsSubbandList.push_back(subband);
		return true;
	}
	else return false;
}

QDataStream& operator<< (QDataStream &out, const DigitalBeam &digiBeam) {
	if (out.status() == QDataStream::Ok) {
		out << digiBeam.itsTarget
			<< digiBeam.itsAngle1
			<< digiBeam.itsAngle2
			<< (quint8) digiBeam.itsDirectionType
			<< (quint8) digiBeam.itsUnits
			<< digiBeam.itsDuration
			<< digiBeam.itsStartTime
			<< digiBeam.itsNrTabRings
			<< digiBeam.itsTabRingSize;

		// subbandList
		const std::vector<unsigned> &subbandList(digiBeam.subbandList());
		out << (quint32) subbandList.size();
		for (std::vector<unsigned>::const_iterator vit = subbandList.begin(); vit != subbandList.end(); ++vit) {
			out << (quint16) *vit;
		}

		// tied array beams
		const std::map<unsigned, TiedArrayBeam> &tiedArrayBeams(digiBeam.tiedArrayBeams());
		out << (quint32) tiedArrayBeams.size();
		for (std::map<unsigned, TiedArrayBeam>::const_iterator vit = tiedArrayBeams.begin(); vit != tiedArrayBeams.end(); ++vit) {
			out << vit->first << vit->second;
		}
	}

	return out;
}

QDataStream& operator>> (QDataStream &in, DigitalBeam &digiBeam) {
	quint16 subband;
	quint8 beamDirType, units;

	in >> digiBeam.itsTarget
	   >> digiBeam.itsAngle1 >> digiBeam.itsAngle2
	   >> beamDirType;
	digiBeam.itsDirectionType = (beamDirectionType) beamDirType;
	in >> units;
	digiBeam.itsUnits = (anglePairs) units;
	in >> digiBeam.itsDuration >> digiBeam.itsStartTime
	   >> digiBeam.itsNrTabRings >> digiBeam.itsTabRingSize;

	// digital beam subband list
	digiBeam.itsSubbandList.clear();
	quint32 nrOfObjects;
	in >> nrOfObjects; // number of subbands
	for (quint32 i = 0; i < nrOfObjects; ++i) {
		in >> subband; // subband number
		digiBeam.itsSubbandList.push_back(subband);
	}

	// tied array beams
	digiBeam.itsTiedArrayBeams.clear();
	unsigned beamNr;
	TiedArrayBeam tiedArrayBeam;
	in >> nrOfObjects;
	for (quint32 i = 0; i < nrOfObjects; ++i) {
		tiedArrayBeam.clear();
		in >> beamNr >> tiedArrayBeam;
		digiBeam.itsTiedArrayBeams[beamNr] = tiedArrayBeam;
	}

	return in;
}

bool DigitalBeam::operator!=(const DigitalBeam & right) const {
	return ((itsTarget != right.target()) ||
		(itsAngle1 != right.angle1()) ||
		(itsAngle2 != right.angle2()) ||
		(itsDirectionType != right.directionType()) ||
		(itsNrTabRings != right.nrTABrings()) ||
		(fabs(itsTabRingSize - right.tabRingSize()) > std::numeric_limits<double>::epsilon()) ||
		(itsDuration != right.duration()) ||
		(itsStartTime != right.startTime()) ||
		(itsSubbandList != right.subbandList()) ||
		(itsTiedArrayBeams != right.tiedArrayBeams()));
}

bool DigitalBeam::setSubbandList(const QString &subbands) {
	bool error(false);
	itsSubbandList = StringList2VectorOfUint(subbands, error);
	return !error;
}

unsigned DigitalBeam::nrIncoherentTABs(void) const {
	unsigned nrIncoherent(0);
	for (std::map<unsigned, TiedArrayBeam>::const_iterator it = itsTiedArrayBeams.begin(); it != itsTiedArrayBeams.end(); ++it) {
		if (!it->second.isCoherent()) ++nrIncoherent;
	}
	return nrIncoherent;
}

unsigned DigitalBeam::nrCoherentTABs(void) const {
	unsigned nrCoherent(0);
	for (std::map<unsigned, TiedArrayBeam>::const_iterator it = itsTiedArrayBeams.begin(); it != itsTiedArrayBeams.end(); ++it) {
		if (it->second.isCoherent()) ++nrCoherent;
	}
	return nrCoherent;
}

void DigitalBeam::clear(void) {
	itsSubbandNotationChange = false;
	itsTarget="";
	itsAngle1.clear();
	itsAngle2.clear();
	itsDirectionType = DIR_TYPE_J2000;
	itsUnits = ANGLE_PAIRS_HMS_DMS;
    itsDuration.clearTime();
    itsStartTime.clearTime();
	itsSubbandList.clear();
	itsTiedArrayBeams.clear();
}
