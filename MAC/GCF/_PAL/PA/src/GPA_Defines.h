//#  GPA_Defines.h: preprocessor definitions of various constants
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

#ifndef GPA_DEFINES_H
#define GPA_DEFINES_H

//#define LOFARLOGGER_PACKAGE "MAC.GCF.PAL.PA.Logger"

#include <GCF/GCF_Defines.h>

class GCFPValue;

enum TPAResult {
  PA_NO_ERROR, 
  PA_UNKNOWN_ERROR,
  PA_SCOPE_ALREADY_REGISTERED,
  PA_PROP_SET_GONE,
  PA_MISSING_PROPS,
  PA_PROP_NOT_VALID,
  PA_UNABLE_TO_LOAD_APC,
  PA_NO_TYPE_SPECIFIED_IN_APC,
  PA_EMPTY_SCOPE,
  PA_SCADA_ERROR,
  PA_SAL_ERROR,
  PA_MACTYPE_UNKNOWN,
  PA_PROP_SET_NOT_EXISTS,
  PA_PROP_SET_ALLREADY_EXISTS,
  PA_DPTYPE_UNKNOWN,
  PA_PS_ALLREADY_EXISTS,
  PA_INTERNAL_ERROR,
  PA_WRONG_STATE,
  PA_PS_NOT_EXISTS
};

typedef struct
{
  string name;
  GCFPValue* pValue;
  bool  defaultSet;
} TAPCProperty;

#endif
