/*
 * task.h
 *
 * Author         : Alwin de Jong
 * e-mail         : jong@astron.nl
 * Revision       : $Revision$
 * Last change by : $Author$
 * Change date	  : $Date$
 * First creation : Dec 6, 2011
 * URL            : $URL: https://svn.astron.nl/ROD/trunk/LOFAR_Scheduler/TiedArrayBeam.h $
 *
 */

#include "lofar_scheduler.h"
#include <QDataStream>

#ifndef TIEDARRAYBEAM_H_
#define TIEDARRAYBEAM_H_

class TiedArrayBeam {
public:
	TiedArrayBeam() :
		itsAngle1(0.0), itsAngle2(0.0), itsIsCoherent(false), itsDispersionMeasure(0.0) { }
	TiedArrayBeam(const double &angle1, const double &angle2, bool coherent, const float &dm) :
		itsAngle1(angle1), itsAngle2(angle2), itsIsCoherent(coherent), itsDispersionMeasure(dm) { }
	virtual ~TiedArrayBeam();

	friend QDataStream& operator>> (QDataStream &in, TiedArrayBeam &tiedArrayBeam);
	friend QDataStream& operator<< (QDataStream &out, const TiedArrayBeam &tiedArrayBeam);

	bool operator!=(const TiedArrayBeam & right) const;
	bool operator==(const TiedArrayBeam & right) const {return (!(*this != right));}

	bool isCoherent(void) const {return itsIsCoherent;}
	const double &angle1(void) const {return itsAngle1;}
	const double &angle2(void) const {return itsAngle2;}
	const double &dispersionMeasure(void) const {return itsDispersionMeasure;}

	void clear(void);
	void setAngle1(const double &angle) {itsAngle1 = angle;}
	void setAngle2(const double &angle) {itsAngle2 = angle;}
	void setCoherent(bool coherent) {itsIsCoherent = coherent;}
	void setDispersionMeasure(const double &dm) {itsDispersionMeasure = dm;}

private:
	double itsAngle1, itsAngle2;
	bool itsIsCoherent;
	double itsDispersionMeasure;
};

#endif /* TIEDARRAYBEAM_H_ */
