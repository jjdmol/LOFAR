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

#include <GCF/GCF_Defines.h>
#include <GCF/GCF_TMProtocols.h>
#define CORE_COMPS_PI_LOGGER  (GCF_LOGGER_ROOT + string(".CoreComps.PI"))

#define PI_STDOUT_LOGGER      (CORE_COMPS_PI_LOGGER + string(".Logger"))

enum { F_SUPERVISORY_PROTOCOL = F_GCF_PROTOCOL};

enum TPIResult {
  PI_NO_ERROR, 
  PI_UNKNOWN_ERROR,
  PI_PA_NOTCONNECTED,
  PI_IS_BUSY,
  PI_SCADA_ERROR,
  PI_SS_BUSY
};

#endif
