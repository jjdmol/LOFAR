//#  APL_Defines.h: preprocessor definitions of various constants
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

#ifndef APL_DEFINES_H
#define APL_DEFINES_H

namespace LOFAR
{
  
namespace APLCommon
{

  enum TLDResult
  {
    LD_RESULT_NO_ERROR = 0, 
    LD_RESULT_UNSPECIFIED,
    LD_RESULT_FILENOTFOUND,
    LD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS,
    LD_RESULT_UNKNOWN_COMMAND,
    LD_RESULT_DISABLED,
    LD_RESULT_LOW_QUALITY,
    LD_RESULT_TIMING_FAILURE,
    LD_RESULT_RANGE_ERROR,     // range error in message parameters
    LD_RESULT_BEAMALLOC_ERROR, // could not allocate beam
    LD_RESULT_BEAMFREE_ERROR,  // could not free beam
    LD_RESULT_SETCLOCKS_ERROR, // error setting td clocks
    LD_RESULT_STARTCAL_ERROR,  // error starting calibration
    LD_RESULT_LOW_PRIORITY,    // a higher priority LD caused a suspend of this LD

  };

  enum 
  {
    RS_DEFECT = -3,
    RS_VERIFY,
    RS_OFFLINE,
    RS_IDLE,
    RS_BUSY,
    RS_SUSPECT_IDLE = 0x40000000
  };

  enum TLogicalDeviceTypes 
  {
    LDTYPE_NO_TYPE = 0, 
    LDTYPE_VIRTUALINSTRUMENT = 1, 
    LDTYPE_VIRTUALTELESCOPE, 
    LDTYPE_ARRAYRECEPTORGROUP, 
    LDTYPE_STATIONRECEPTORGROUP, 
    LDTYPE_ARRAYOPERATIONS, 
    LDTYPE_STATIONOPERATIONS, 
    LDTYPE_VIRTUALBACKEND, 
    LDTYPE_MAINTENANCEVI,
    LDTYPE_OBSERVATION,
    LDTYPE_VIRTUALROUTE
  };

  enum TSDResult
  {
    SD_RESULT_NO_ERROR = 0, 
    SD_RESULT_UNSPECIFIED_ERROR, 
    SD_RESULT_UNSUPPORTED_LD, 
    SD_RESULT_FILENOTFOUND,
    SD_RESULT_PARAMETERNOTFOUND,
    SD_RESULT_INCORRECT_NUMBER_OF_PARAMETERS,
    SD_RESULT_UNKNOWN_COMMAND,
    SD_RESULT_ALREADY_EXISTS,
    SD_RESULT_LD_NOT_FOUND,
    SD_RESULT_WRONG_STATE,
    SD_RESULT_SHUTDOWN,
    SD_RESULT_WRONG_VERSION,
  };

  #define IS_BUSY(s) ((s > 0) && (s != RS_SUSPECT_IDLE))
  #define IS_SUSPECT(s) (s >= RS_SUSPECT_IDLE)
  #define IS_IDLE(s) ((s == 0) || (s == RS_SUSPECT_IDLE))
  #define MAKE_SUSPECT(s) (s |= RS_SUSPECT_IDLE)
  #define MAKE_UNSUSPECT(s) (s &= ~RS_SUSPECT_IDLE)

};

};

#endif
