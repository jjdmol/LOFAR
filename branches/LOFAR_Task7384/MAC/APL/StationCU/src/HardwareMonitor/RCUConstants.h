//#  RCUConstants.h: Mask and offset for decomposing the raw RCUsetting value
//#
//#  Copyright (C) 2007
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

#ifndef STATIONCU_RCUCONSTANTS_H
#define STATIONCU_RCUCONSTANTS_H

namespace LOFAR {
  namespace StationCU {

// NOTE: these constants should match the real mapping that is defined in RCUSettings.h
//		 in the PIC/RSP_Protocol directory.
static const uint32	DELAY_MASK			= 0x0000007F;
static const uint32	INPUT_ENABLE_MASK	= 0x00000080;
static const uint32	LBL_ANT_POWER_MASK	= 0x00000100;
static const uint32	LBH_ANT_POWER_MASK	= 0x00000200;
static const uint32	HBA_ANT_POWER_MASK	= 0x00000400;
static const uint32	USE_LB_MASK			= 0x00000800;
static const uint32	HB_FILTER_MASK		= 0x00003000;
static const uint32	HB_FILTER_OFFSET	= 12;
static const uint32	LB_POWER_MASK		= 0x00004000;
static const uint32	HB_POWER_MASK		= 0x00008000;
static const uint32	ADC_POWER_MASK		= 0x00010000;
static const uint32	USE_LBH_MASK		= 0x00020000;
static const uint32	LB_FILTER_MASK		= 0x00040000;
static const uint32	LB_FILTER_OFFSET	= 18;
static const uint32	ATT_MASK			= 0x00F80000;
static const uint32	ATT_OFFSET			= 19;

#define PN_RSP_AP_VERSION_MASK		"AP%d.version"

  } // namespace StationCU
} // namespace LOFAR

#endif
