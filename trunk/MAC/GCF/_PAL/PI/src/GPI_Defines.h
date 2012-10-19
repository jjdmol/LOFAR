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

namespace LOFAR 
{
 namespace GCF 
 {
  namespace PAL
  {

const string PI_TASK_NAME("GCF-PI");
const string PI_CEPPLS_TASK_NAME("GCF-PI-CEP");
const string PI_RTCPLS_TASK_NAME("GCF-PI-RTC");

const string PI_LINKPS   = "__pa_PiLinkPS";
const string PI_UNLINKPS = "__pa_PiUnlinkPS";
 
  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
