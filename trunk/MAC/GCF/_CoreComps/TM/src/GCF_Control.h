//#  GCF_Control.h: Include this header file when programming tasks.
//#              It includes everything that you need.
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

//
// Each file is guarded with the multiple inclusion guard as
// used in the files. The guards in this file prevent opening and
// reading the file when it has already been included.
// This speeds up compilation.
//

#ifndef GTM_CONTROL_H
#define GTM_CONTROL_H

#ifndef GCF_PORT_H
#include <TM/PortInterface/GCF_Port.h>
#endif

#ifndef GCF_PORTINTERACE_H
#include <TM/PortInterface/GCF_PortInterface.h>
#endif

#ifndef GCF_TASK_H
#include <TM/GCF_Task.h>
#endif

#ifndef GCF_FSM_H
#include <TM/GCF_Fsm.h>
#endif

#ifndef GCF_PROTOCOLS_H
#include <TM/GCF_TMProtocols.h>
#endif

#ifndef GCF_PEERADDR_H
#include <TM/PortInterface/GCF_PeerAddr.h>
#endif

#ifndef GCF_EVENT_H
#include <TM/GCF_Event.h>
#endif

#endif /* GCF_CONTROL_H */
