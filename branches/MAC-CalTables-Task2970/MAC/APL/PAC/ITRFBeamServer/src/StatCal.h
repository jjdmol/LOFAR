//#  StatCal.h: interface of the StatCal class
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//#
//#  This program is free software; you can redistribute it and/or modify
//#  it under the terms of the GNU General Public License as published by
//#  the Free Software Foundation; either version 2 of the License, or
//#  (at your option) any later version.
//#
//#  This program is distributed in the hope that it will be useful,
//#  but WITHOUT ANY WARRANTY; without even the implied warranty of
//#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//#  GNU General Public License for more details.
//#
//#  You should have received a copy of the GNU General Public License
//#  along with this program; if not, write to the Free Software
//#  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//#
//#  $Id: StatCal.h  $

#ifndef STATCAL_H_
#define STATCAL_H_

#include <lofar_config.h>
#include <Common/LofarConstants.h>
#include <Common/lofar_complex.h>
#include <time.h>

#include <blitz/array.h>

namespace LOFAR {
  namespace BS {

// Class representing static calibration data
class StatCal {
public:

	// constructor and destructor
	explicit StatCal(uint rcumode, uint nrRSPBoards);
	~StatCal();

	std::complex<double> calFactor(uint	rcuNr, uint subbandNr) const;
	bool	isValid() const { return (itsIsValid); }

	// operator()
	const blitz::Array<std::complex<double>,3>& operator()() const;

	// operator <<
	ostream& print(ostream& os) const;

private:
	// Don't allow copying this object.
	StatCal();
	StatCal (const StatCal&);            // not implemented
	StatCal& operator= (const StatCal&); // not implemented

	// helper functions
	bool _readData       (uint	mode);
	bool _readHeaderInfo (FILE*	file);
	bool _checkHeaderInfo(uint	mode) const;
	
	uint 	itsNantennas;
	uint 	itsNpols;
	uint 	itsNsubbands;
	uint 	itsMode;
	string 	itsFileName;
	bool	itsIsValid;

	class headerinfo {
	public:
		headerinfo() : mode(-1) {};
		// datamembers
		string	station;
		int		mode;
		string	source;
		string	date;
		string	calVersion;
		string	calName;
		string	calDate;
		string	calPPSdelay;
		string	comment;
	};
	headerinfo		itsHI;

	// two 
	blitz::Array<std::complex<double>, 3> itsStaticCalibration;  // 
};

//# -------------------- inline functions --------------------
//# operator()
inline const blitz::Array<std::complex<double>, 3>& StatCal::operator()() const
{
	return (itsStaticCalibration);
}

//# operator<<
inline ostream& operator<< (ostream& os, const StatCal&  sc)
{
	return (sc.print(os));
}

  } //# namepsace BS
} //# namespace LOFAR

#endif /* STATCAL_H_ */
