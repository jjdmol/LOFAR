//#  GCF_Defines.h: preprocessor definitions of various constants
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

#ifndef GCF_DEFINES_H
#define GCF_DEFINES_H

#include <LofarLogger/LofarLogger.h>

#define GCF_LOGGER_ROOT       (MAC_LOGGER_ROOT + string(".GCF"))

enum TGCFResult {
  GCF_NO_ERROR = 1, 
  GCF_UNKNOWN_ERROR = 0,
  GCF_PML_ERROR, 
};

typedef unsigned short TAccessMode;

typedef struct
{
  char          propName[];
  unsigned int  type;
  TAccessMode                 accessMode;
}
TProperty;

typedef struct
{
  unsigned int  nrOfProperties;
  char          scope[];
  TProperty*    properties;
}
TPropertySet;
#endif
