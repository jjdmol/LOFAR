//#  StationControlDefines.h: preprocessor definitions of various constants
//#
//#  Copyright (C) 2002-2003
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

#ifndef STATIONCONTROLDEFINES_H
#define STATIONCONTROLDEFINES_H

namespace LOFAR {
  namespace StationCU {

// next three line should be defined elsewhere because we are not the owner.
//#define	PSN_STATION_CLOCK	"LOFAR_PermSW_MACScheduler"
//#define PST_STATION_CLOCK	"MACScheduler"
//#define PN_SC_CLOCK			"OTDB.pollinterval"
#define	PSN_STATION_CLOCK	"LOFAR_PIC_StationClock"
#define PST_STATION_CLOCK	"StationClock"
#define PN_SC_CLOCK			"clock"

// next lines should be defined somewhere in Common.
#define PVSSNAME_FSM_STATE			"state"
#define PVSSNAME_FSM_ERROR			"error"


}; // StationCU
}; // LOFAR

#endif
