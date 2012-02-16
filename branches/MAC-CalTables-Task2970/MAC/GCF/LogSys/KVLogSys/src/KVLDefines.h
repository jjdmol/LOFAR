//#  KVDefines.h: preprocessor definitions of various constants
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

#ifndef KVDEFINES_H
#define KVDEFINES_H

#include <GCF/GCF_Defines.h>


namespace LOFAR 
{
 namespace GCF 
 {
  namespace LogSys 
  {

#define MAX_EVENTS_BUFF_SIZE 1400
#define MAX_NR_OF_EVENTS 250
#define MAX_NR_OF_RETRY_MSG 10
#define TO_DISCONNECTED 3600.0 // == 1 hour
#define TO_TRY_RECONNECT 5.0

const string KVL_CLIENT_TASK_NAME("GCF-KVLC");
const string KVL_DAEMON_TASK_NAME("GCF-KVLD");
const string KVL_MASTER_TASK_NAME("GCF-KVLM");


  } // namespace PAL
 } // namespace GCF
} // namespace LOFAR

#endif
