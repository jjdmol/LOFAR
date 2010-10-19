//  tRTDButilities.h: Definition of the DPservice task class.
//
//  Copyright (C) 2007
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, seg@astron.nl
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  $Id: tDPservice.h 10538 2007-10-03 15:04:43Z overeem $
//

#ifndef _RTDBCOMMON_TRTDBUTILITIES_H
#define _RTDBCOMMON_TRTDBUTILITIES_H

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
 namespace APL {
  namespace RTDBCommon {

class tRTDButil : public GCFTask
{
public:
	tRTDButil (const string& name);
	virtual ~tRTDButil();

	GCFEvent::TResult doTest	(GCFEvent& e, GCFPortInterface& p);

private:
	GCFTimerPort*		itsTimerPort;
};

  } // namespace RTDBCommon
 } // namespace APL
} // namespace LOFAR

#endif
