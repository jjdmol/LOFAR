/*
 * task.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Dec 6, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/TiedArrayBeam.cpp $
 *
 */

#include "TiedArrayBeam.h"


TiedArrayBeam::~TiedArrayBeam() {
	// TODO Auto-generated destructor stub
}

QDataStream& operator>> (QDataStream &in, TiedArrayBeam &tiedArrayBeam) {
	in >> tiedArrayBeam.itsAngle1 >> tiedArrayBeam.itsAngle2 >> tiedArrayBeam.itsIsCoherent >> tiedArrayBeam.itsDispersionMeasure;
	return in;
}

QDataStream& operator<< (QDataStream &out, const TiedArrayBeam &tiedArrayBeam) {
	out << tiedArrayBeam.itsAngle1 << tiedArrayBeam.itsAngle2 << tiedArrayBeam.itsIsCoherent << tiedArrayBeam.itsDispersionMeasure;
	return out;
}

bool TiedArrayBeam::operator!=(const TiedArrayBeam & right) const {
	return ((itsAngle1 != right.angle1()) ||
		(itsAngle2 != right.angle2()) ||
		(itsIsCoherent != right.isCoherent()) ||
		(itsDispersionMeasure != right.dispersionMeasure()));
}

void TiedArrayBeam::clear(void) {
	itsAngle1 = 0.0;
	itsAngle2 = 0.0;
	itsIsCoherent = false;
	itsDispersionMeasure = 0.0;
}
