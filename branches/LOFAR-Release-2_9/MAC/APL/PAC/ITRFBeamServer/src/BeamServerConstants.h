//#  BeamServerConstants.h: Some constants used in several places of the BS code.
//#
//#  Copyright (C) 2007
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef BEAMSERVER_BEAMSERVERCONSTANTS_H
#define BEAMSERVER_BEAMSERVERCONSTANTS_H

#define SPEED_OF_LIGHT_MS 		299792458.0		// speed of light in meters/sec

#define LEADIN_TIME 	 		3				// time it takes before a pointing is active

#define HBA_INTERVAL 			300				// normal interval between two HBA-delay settings
#define HBA_MIN_INTERVAL		4				// absolute minimum interval for the HBAs
#define DIG_INTERVAL 			1				// how often the weights are sent
#define DIG_COMPUTE_INTERVAL  	1				// how often the weights are calculated

#endif

