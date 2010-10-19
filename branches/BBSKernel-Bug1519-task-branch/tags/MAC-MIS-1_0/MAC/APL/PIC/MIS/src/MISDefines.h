//#  MISDefines.h: preprocessor definitions of various constants
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

#ifndef MISDEFINES_H
#define MISDEFINES_H

#include <GCF/GCF_Defines.h>

namespace LOFAR 
{
 namespace AMI
 {
const uint8 MIS_MAJOR_VER = 1; 
const uint8 MIS_MIDOR_VER = 0;
const uint8 MIS_MINOR_VER = 0;

const string MIS_TASK_NAME("APL-MIS");
const string MIS_CONF("MIS.conf");
const string MIS_PORT_NAME("client");

const string MISS_TASK_NAME("APL-MISS");
const string MISS_PORT_NAME("misd-session");

const string MISD_TASK_NAME("APL-MISD");
const string MISD_PORT_NAME("server");

const string MIS_RSP_PORT_NAME("rspclient");

 } // namespace AMI
} // namespace LOFAR

#endif
