//#  GTM_Control.h: Include this header file when programming tasks.
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

#ifndef _FPORT_H_
#include "FPort.h"
#endif

#ifndef _FREPLICATEDPORT_H_
#include "FRepPort.h"
#endif

#ifndef _FPORTINTERACE_H_
#include "FPortInterface.h"
#endif

#ifndef _FTASK_H_
#include "FTask.h"
#endif

#ifndef _FNAMESERVICE_H_
#include "FNameService.h"
#endif

#ifndef _FTOPOLOGYSERVICE_H_
#include "FTopologyService.h"
#endif

#ifndef _FSM_H_
#include "Fsm.h"
#endif

#ifndef _FPROTOCOLS_H_
#include "FProtocols.h"
#endif

#ifndef _FPEERADDR_H_
#include "FPeerAddr.h"
#endif

#ifndef _FEVENT_H_
#include "FEvent.h"
#endif

#ifndef _FDEFINES_H_
#include "FDefines.h"
#endif

#endif /* _FCONTROL_H_ */
