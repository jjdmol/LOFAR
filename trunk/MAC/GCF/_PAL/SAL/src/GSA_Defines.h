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

enum TSAResult 
{
  SA_NO_ERROR, 
  SA_UNKNOWN_ERROR, 
  SA_PROPNAME_MISSING,
  SA_VARIABLE_WRONG_TYPE,
  SA_DPTYPE_UNKNOWN,
  SA_MACTYPE_UNKNOWN,
  SA_CREATEPROP_FAILED,
  SA_DELETEPROP_FAILED,
  SA_SUBSCRIBEPROP_FAILED,
  SA_UNSUBSCRIBEPROP_FAILED,
  SA_SETPROP_FAILED,
  SA_GETPROP_FAILED,
  SA_SCADA_NOT_AVAILABLE, 
  SA_DIFFERENT_TYPES,
  SA_PROP_DOES_NOT_EXIST,
  SA_PROP_ALREADY_EXIST,
  SA_VALUESTRING_NOT_VALID,
  SA_MACTYPE_MISMATCH,
  SA_DP_ALREADY_EXISTS
};

#define PARAM_PVSS_CMDLINE "mac.%s.pvss.cmdline"
#define PARAM_DEFAULT_PVSS_CMDLINE "mac.controller.pvss.cmdline"
#endif
