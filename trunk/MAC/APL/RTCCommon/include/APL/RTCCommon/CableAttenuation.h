//#  CableAttenuation.h: Interface class for the Attenuation.conf file
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

#ifndef LOFAR_RTCCOMMON_CABLEATTENUATION_H
#define LOFAR_RTCCOMMON_CABLEATTENUATION_H

// \file
// Interface class for the Attenuation.conf file.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <blitz/array.h>
#include <Common/lofar_fstream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>


namespace LOFAR {
  namespace RTC{

// \addtogroup RTC
// @{

//# Forward Declarations
//class forward;

// The CableAttenuation class is an interface for the Attenuation.conf file.
//It reads in the file and stores the values in its datamembers. Defines some
//useful functions for accessing the data.

class CableAttenuation
{
public:
	CableAttenuation(const string&	filename);
	~CableAttenuation();

	// Returns the attenuation in dB for the given cable length and rcumode.
	float	getAttenuation(int	cableLength, int	rcuMode) const;

private:
	// Default construction and Copying is not allowed.
	CableAttenuation();
	CableAttenuation (const CableAttenuation& that);
	CableAttenuation& operator= (const CableAttenuation& that);

	//# private functions
	// Converts a cablelength into an index in the itsAtts array.
	int CableAttenuation::cableLen2Index(int	cableLen) const;

	//# Data members
	blitz::Array<int, 1>	itsCableLengths;
	blitz::Array<float,2>	itsAtts;
};

// @}

  } // namespace RTC
} // namespace LOFAR

#endif
