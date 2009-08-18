//#  AntennaMapper.h: one line description
//#
//#  Copyright (C) 2008
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
//#  $Id$

#ifndef LOFAR_APLCOMMON_ANTENNAMAPPER_H
#define LOFAR_APLCOMMON_ANTENNAMAPPER_H

// \file
// one line description.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!


namespace LOFAR {
  namespace APLCommon {
// \addtogroup APLCommon
// @{

//# Forward Declarations
//class forward;


// Description of class.
class AntennaMapper
{
public:
	// define some constants
	static const int AT_LBA = 0;		// AntennaType LBA
	static const int AT_HBA = 1;		// AntennaType HBA

	static const int RI_LBL = 0;		// RcuInput LBL
	static const int RI_LBH = 1;		// RcuInput LBH
	static const int RI_HBA = 2;		// RcuInput HBA
	static const int RI_MAX_INPUTS = 3;

	AntennaMapper(int	nrRCUs, int	nrLBAs, int	nrHBAs) :
		itsRCUs(nrRCUs), itsLBAs(nrLBAs), itsHBAs(nrHBAs) {}
	~AntennaMapper() {}

	// Convert an antennanumber to the RCU the X-pole or Y-pole is connected
	int	XRCU(int	antNr) const;
	int	YRCU(int	antNr) const;

	// Convert an antennanumber to the RCUinputType the antenna is connected.
	int	RCUinput(int	antNr, int antennaType) const;

private:
	// Default constructor.
	AntennaMapper();
	// Copying is not allowed.
	AntennaMapper (const AntennaMapper& that);
	AntennaMapper& operator= (const AntennaMapper& that);

	//# Data members
	int		itsRCUs;
	int		itsLBAs;
	int		itsHBAs;
};

// ----- inline functions -----

inline int	AntennaMapper::XRCU(int	antNr) const
{
	return ((antNr <= itsRCUs/2) ? 2*antNr : 2*antNr+1 - itsRCUs);
}

inline int AntennaMapper::YRCU(int	antNr) const
{
	return ((antNr <= itsRCUs/2) ? 2*antNr+1 : 2*antNr - itsRCUs);
}

inline int AntennaMapper::RCUinput(int	antNr, int antType) const
{
	if (antType == AT_HBA)
		return (RI_HBA);
	if (antNr < itsRCUs/2) 
		return (RI_LBH);
	return (RI_LBL);

	// Using this oneliner gives unresolved references to the RI_xxx values!!!
//	return ((antType == AT_HBA) ? RI_HBA : (antNr < itsRCUs/2) ? RI_LBH : RI_LBL);
}

// @}

  } // namespace APLCommon
} // namespace LOFAR

#endif
