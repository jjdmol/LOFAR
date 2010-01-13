//#  AntennaPos.h: Class to manage the antenna subsets.
//#
//#  Copyright (C) 2009
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

#ifndef LOFAR_APLCOMMON_ANTENNAPOS_H
#define LOFAR_APLCOMMON_ANTENNAPOS_H

// \file AntennaPos.h
// Class to manage the antenna subsets.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <blitz/array.h>

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace APLCommon {

// \addtogroup package
// @{

//# --- Forward Declarations ---
//# classes mentioned as parameter or returntype without virtual functions.


// This class represents an antenna array. The LOFAR remote station will initially
// have two anntena arrays. One for the low-band antennas, and one for the high-band
// antennas. Alternative configurations of the antennas are described in AntennaSets.
class AntennaPos
{
public:
	explicit AntennaPos (const string&	filename);
	virtual ~AntennaPos();

	// all about the bits
	// [antNr, pol, xyz]
	const blitz::Array<double, 3>& LBAAntPos()	const	{ return itsLBAAntPos; }
	const blitz::Array<double, 3>& HBAAntPos()	const	{ return itsHBAAntPos; }

	// [rcu, xyz]
	const blitz::Array<double, 2>& LBARCUPos()	const	{ return itsLBARCUPos; }
	const blitz::Array<double, 2>& HBARCUPos()	const	{ return itsHBARCUPos; }

	// [xyz]
	const blitz::Array<double, 1>& LBACentre()	const	{ return itsLBACentre; }
	const blitz::Array<double, 1>& HBACentre()	const	{ return itsHBACentre; }

	// [rcu, xyz]
	const blitz::Array<double, 1>& LBARCULengths()	const	{ return itsLBARCULengths; }
	const blitz::Array<double, 1>& HBARCULengths()	const	{ return itsHBARCULengths; }

	int		nrLBAs() const { return itsLBAAntPos.extent(blitz::firstDim); }
	int		nrHBAs() const { return itsHBAAntPos.extent(blitz::firstDim); }

private:
	// Copying is not allowed
	AntennaPos();
	AntennaPos(const AntennaPos&	that);
	AntennaPos& operator=(const AntennaPos& that);

	//# --- Datamembers ---
	blitz::Array<double,1>		itsLBACentre;	// [ (x,y,z) ]
	blitz::Array<double,1>		itsHBACentre;	// [ (x,y,z) ]

	blitz::Array<double,2>		itsLBARCUPos;	// [ rcuNr, (x,y,z) ]
	blitz::Array<double,2>		itsHBARCUPos;	// [ rcuNr, (x,y,z) ]

	blitz::Array<double,3>		itsLBAAntPos;	// [ antNr, pol, (x,y,z) ]
	blitz::Array<double,3>		itsHBAAntPos;	// [ antNr, pol, (x,y,z) ]

	// during calculations we often need the length of the vectors.
	blitz::Array<double,1>		itsLBARCULengths;	// [ len ]
	blitz::Array<double,1>		itsHBARCULengths;	// [ len ]
};


// @}
  } // namespace APLCommon
} // namespace LOFAR

#endif
