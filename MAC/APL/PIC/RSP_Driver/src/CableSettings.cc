//#  CableSettings.cc: Global station settings
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
//#  $Id$

//# Always #include <lofar_config.h> first!
#include <lofar_config.h>

//# Includes
#include <Common/LofarLogger.h>
#include <Common/LofarConstants.h>
#include "StationSettings.h"
#include "CableSettings.h"

namespace LOFAR {
  namespace RSP {

//
// Initialize singleton
//
CableSettings* CableSettings::theirCableSettings = 0;

CableSettings* CableSettings::instance()
{
	ASSERTSTR(theirCableSettings, "Trying to access the CableSetting information before initialisation is done");
	return (theirCableSettings);
}

//
// CableSettings(cableObject)
//
CableSettings::CableSettings(const RCUCables*	cableObject)
{
	int	nrRCUs = StationSettings::instance()->nrRcus();
	itsAtts.resize(nrRCUs, NR_RCU_MODES+1);
	itsDelays.resize(nrRCUs, NR_RCU_MODES+1);

	// Construct arrays cantaining with the smallest Atts and delays that are possible.
	for (int	mode = 0; mode <= NR_RCU_MODES; mode++) {
		float	largestAtt   = cableObject->getLargestAtt(mode);
		float	largestDelay = cableObject->getLargestDelay(mode);
		for (int	rcu = 0; rcu < nrRCUs; rcu++) {
			itsAtts  (rcu, mode) = largestAtt   - cableObject->getAtt  (rcu,mode);
			itsDelays(rcu, mode) = largestDelay - cableObject->getDelay(rcu,mode);
		}
	}

	theirCableSettings = this;

	LOG_DEBUG_STR("Attenuations: rcus x modes");
	LOG_DEBUG_STR(itsAtts);
	LOG_DEBUG_STR("Delays: rcus x modes");
	LOG_DEBUG_STR(itsDelays);
}

  } // namespace PACKAGE
} // namespace LOFAR
