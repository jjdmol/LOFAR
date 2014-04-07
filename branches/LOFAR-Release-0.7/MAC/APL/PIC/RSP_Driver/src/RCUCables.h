//#  RCUCables.h: Interface class for the cable characteristics.
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

#ifndef LOFAR_APPLCOMMON_RCUCABLES_H
#define LOFAR_APPLCOMMON_RCUCABLES_H

// \file
// Interface class for the cable characteristics.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!

//# Includes
#include <blitz/array.h>
#include <Common/lofar_fstream.h>
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include <Common/LofarLocators.h>
#include <Common/StringUtil.h>
#include "CableAttenuation.h"

namespace LOFAR {

// \addtogroup ApplCommon
// @{

//# Forward Declarations
//class forward;

// The RCUCables class is an interface for the Attenuation.conf file.
//It reads in the file and stores the values in its datamembers. Defines some
//useful functions for accessing the data.

class RCUCables
{
public:
	RCUCables(const string&	attFilename, const string&	delayFilename);
	~RCUCables();

	// Returns the attenuation in dB for the given rcu when operation in the given rcumode.
	float	getAtt  (int	rcuNr, int	rcuMode) const;

	// Returns the delay in ns for the given rcu when operation in the given rcumode.
	float	getDelay(int	rcuNr, int	rcuMode) const;

	// Returns the largest attenuation in dB when operation in the given rcumode.
	float	getLargestAtt  (int	rcuMode) const;

	// Returns the largest delay in ns when operation in the given rcumode.
	float	getLargestDelay(int	rcuMode) const;

private:
	// Default construction and Copying is not allowed.
	RCUCables();
	RCUCables (const RCUCables& that);
	RCUCables& operator= (const RCUCables& that);

	static const int MAX_RCU_INPUTS = 3;
	static const int MAX_RCU_MODE   = 7;

	//# Data members
	float		itsLargestLBAdelay;
	float		itsLargestHBAdelay;
	int			itsLargestLBAlen;
	int			itsLargestHBAlen;

	CableAttenuation*			itsCableAtts;
	blitz::Array<int,  2>		itsCableLengths;
	blitz::Array<float,2>		itsCableDelays;
};

// @}

} // namespace LOFAR

#endif
