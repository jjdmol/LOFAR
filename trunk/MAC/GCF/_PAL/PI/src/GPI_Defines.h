//#  GPI_Defines.h: preprocessor definitions of various constants
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

#ifndef GPI_DEFINES_H
#define GPI_DEFINES_H

//#define LOFARLOGGER_PACKAGE "MAC.GCF.PAL.PI.Logger"

#include <GCF/GCF_Defines.h>

enum TPIResult 
{
  PI_NO_ERROR, 
  PI_UNKNOWN_ERROR,
  PI_WRONG_STATE,
  PI_PS_GONE,
  PI_MISSING_PROPS,
  PI_PROP_SET_NOT_EXISTS,
  PI_PROP_SET_ALLREADY_EXISTS,
  PI_DPTYPE_UNKNOWN,
  PI_INTERNAL_ERROR,
  PI_PA_INTERNAL_ERROR,
  PI_PA_NOTCONNECTED,
  PI_PROP_NOT_VALID,
  PI_EMPTY_SCOPE,
  PI_MACTYPE_UNKNOWN,  
};

#endif
