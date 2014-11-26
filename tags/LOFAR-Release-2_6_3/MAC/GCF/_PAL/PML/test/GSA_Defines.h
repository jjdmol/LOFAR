//#  GSA_Defines.h: preprocessor definitions of various constants
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

#ifndef GSA_DEFINES_H
#define GSA_DEFINES_H

#include <GCF/GCF_Defines.h>

namespace LOFAR {
 namespace GCF {
  namespace PAL {

enum TSAResult 
{
  SA_NO_ERROR = 0, 				// 0
  SA_UNKNOWN_ERROR, 			// 1
  SA_PROPNAME_MISSING,			// 2
  SA_VARIABLE_WRONG_TYPE,		// 3
  SA_DPTYPE_UNKNOWN,			// 4
  SA_MACTYPE_UNKNOWN,			// 5
  SA_CREATEPROP_FAILED,			// 6
  SA_DELETEPROP_FAILED,			// 7
  SA_SUBSCRIBEPROP_FAILED,		// 8
  SA_UNSUBSCRIBEPROP_FAILED,	// 9
  SA_SETPROP_FAILED,			// 10
  SA_GETPROP_FAILED,			// 11
  SA_QUERY_SUBSC_FAILED,		// 12
  SA_QUERY_UNSUBSC_FAILED,		// 13
  SA_SCADA_NOT_AVAILABLE, 		// 14
  SA_PROP_DOES_NOT_EXIST,		// 15
  SA_PROP_ALREADY_EXIST,		// 16
  SA_VALUESTRING_NOT_VALID,		// 17
  SA_MACTYPE_MISMATCH,			// 18
};

#define PARAM_PVSS_CMDLINE "mac.%s.pvss.cmdline"
#define PARAM_DEFAULT_PVSS_CMDLINE "mac.controller.pvss.cmdline"

  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR
#endif
