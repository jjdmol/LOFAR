//#  MACScheduler_Defines.h: preprocessor definitions of various constants
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

#ifndef MACScheduler_DEFINES_H
#define MACScheduler_DEFINES_H

namespace LOFAR
{
  
namespace GSO
{

#define MS_CONFIG_PREFIX            "mac.apl.ams."
#define MS_TASKNAME                 "MACScheduler"

#define MS_STATE_STRING_INITIAL     "Initial"
#define MS_STATE_STRING_IDLE        "Idle"

#define MS_PROPSET_NAME             "GSO_MACScheduler"
#define MS_PROPSET_TYPE             "TAplMacScheduler"
#define MS_PROPNAME_COMMAND         "command"
#define MS_PROPNAME_STATUS          "status"

#define MS_LOGICALSEGMENT_PROPSET_BASENAME  "PAC_WAN_Segments_"
#define MS_LOGICALSEGMENT_PROPSET_TYPE      "TWanLogicalSegment"
#define MS_LOGICALSEGMENT_PROPNAME_CAPACITY       "Capacity"
#define MS_LOGICALSEGMENT_PROPNAME_ALLOCATED      "AllocatedBW"
#define MS_LOGICALSEGMENT_PROPNAME_ACTUALTRAFFIC  "ActualTraffic"
#define MS_LOGICALSEGMENT_PROPNAME_CHANGEALLOCATED "changeAllocatedBW"

#define MS_COMMAND_SCHEDULE         "SCHEDULE"
#define MS_COMMAND_UPDATESCHEDULE   "UPDATESCHEDULE"
#define MS_COMMAND_CANCELSCHEDULE   "CANCELSCHEDULE"

};

};

#endif
