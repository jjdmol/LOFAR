//#  AVTDefines.h: common defines for the AVT package
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

#ifndef AVTDefines_H
#define AVTDefines_H

namespace AVT
{
  
const char PARAM_PREPARETIME[]    = "mac.apl.avt.PREPARETIME";
const char PARAM_BEAMSERVERPORT[] = "mac.apl.avt.BEAMSERVERPORT";
const char PARAM_APC[]            = "mac.apl.avt.APC_";

#define AVT_N_BEAMLETS 128

#define LD_COMMAND_SCHEDULE           "SCHEDULE"
#define LD_COMMAND_CANCEL             "CANCEL"
#define LD_COMMAND_RELEASE            "RELEASE"
#define LD_COMMAND_PREPARE            "PREPARE"
#define LD_COMMAND_SUSPEND            "SUSPEND"
#define LD_COMMAND_RESUME             "RESUME"
#define LD_COMMAND_MAINTENANCE        "MAINTENANCE"
#define LD_COMMAND_CANCELMAINTENANCE  "CANCELMAINTENANCE"

#define DIRECTIONTYPE_J2000   "J2000"
#define DIRECTIONTYPE_AZEL    "AZEL"
#define DIRECTIONTYPE_LMN     "LMN"

typedef enum
{
  LOGICALDEVICE_STATE_IDLE=0,
  LOGICALDEVICE_STATE_CLAIMING,
  LOGICALDEVICE_STATE_CLAIMED,
  LOGICALDEVICE_STATE_PREPARING,
  LOGICALDEVICE_STATE_SUSPENDED,
  LOGICALDEVICE_STATE_ACTIVE,
  LOGICALDEVICE_STATE_RELEASING,
  LOGICALDEVICE_STATE_RELEASED
} TLogicalDeviceState;

const char LD_STATE_STRING_INITIAL[]    = "Initial";
const char LD_STATE_STRING_IDLE[]       = "Idle";
const char LD_STATE_STRING_CLAIMING[]   = "Claiming";
const char LD_STATE_STRING_CLAIMED[]    = "Claimed";
const char LD_STATE_STRING_PREPARING[]  = "Preparing";
const char LD_STATE_STRING_SUSPENDED[]  = "Suspended";
const char LD_STATE_STRING_ACTIVE[]     = "Active";
const char LD_STATE_STRING_RELEASING[]  = "Releasing";
const char LD_STATE_STRING_RELEASED[]   = "Released";

#endif
