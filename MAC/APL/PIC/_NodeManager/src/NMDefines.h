//#  NMDefines.h: preprocessor definitions of various constants
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

#ifndef NMDEFINES_H
#define NMDEFINES_H

#include <GCF/GCF_Defines.h>

namespace LOFAR 
{
 namespace ANM 
 {

const string NM_TASK_NAME("APL-NM");
const string NM_CONF("NodeManager.conf");
const string NM_PORT_NAME("client");

const string NMC_TASK_NAME("APL-NMC");
const string NMC_PORT_NAME("nmd-client");

const string NMD_TASK_NAME("APL-NMD");
const string NMD_PORT_NAME("server");

 } // namespace ANM
} // namespace LOFAR

#endif
