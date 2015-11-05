//#  CableSettings.h: Global cable settings
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

#ifndef LOFAR_RSP_CABLESETTINGS_H
#define LOFAR_RSP_CABLESETTINGS_H

// \file CableSettings.h
// Global cable settings

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <blitz/array.h>
#include <Common/LofarTypes.h>
#include "RCUCables.h"

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace RSP {

// \addtogroup RSP
// @{

//# forward declaration

// class_description
// Global class holding the cable delays and attenuations for every cable in every rcumode.
class CableSettings
{
public:
	CableSettings (const RCUCables*	CableObject);
	~CableSettings() ;

	static CableSettings* instance();

	// Returns the attenuation array
	inline blitz::Array<float, 2>&	getAtts()	{ return itsAtts; }
	inline float getAtt(int rcu, int rcumode)	{ return itsAtts(rcu,rcumode); }

	// Returns the delays array
	inline blitz::Array<float, 2>&	getDelays()	{ return itsDelays; }
	inline float getDelay(int rcu, int rcumode)	{ return itsDelays(rcu,rcumode); }

private:
	// Default construction and copying is not allowed
	CableSettings();
	CableSettings(const CableSettings&	that);
	CableSettings& operator=(const CableSettings& that);

	//# --- Datamembers ---
	blitz::Array<float,	2>	itsAtts;
	blitz::Array<float,	2>	itsDelays;

	static CableSettings* theirCableSettings;
};

// @}
  } // namespace RSP
} // namespace LOFAR

#endif
