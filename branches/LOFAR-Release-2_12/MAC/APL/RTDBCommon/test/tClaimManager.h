//  tClaimManager.h: Definition of the DPservice task class.
//
//  Copyright (C) 2008
//  ASTRON (Netherlands Foundation for Research in Astronomy)
//  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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
//  $Id$
//

#ifndef _RTDBCOMMON_TCLAIMMANAGER_H
#define _RTDBCOMMON_TCLAIMMANAGER_H

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
 namespace APL {
  namespace RTDBCommon {

class MgrTest : public GCF::TM::GCFTask
{
public:
	MgrTest (const string& name);
	virtual ~MgrTest();

	GCFEvent::TResult doSingleTest	(GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult doMultipleTest(GCFEvent& e, GCFPortInterface& p);

private:
	GCF::TM::GCFTimerPort*		itsTimerPort;
	GCF::TM::GCFITCPort*		itsMsgPort;
	ClaimMgrTask*				itsClaimMgrTask;
	int							itsAnswers2Xpect;
};

  } // namespace RTDBCommon
 } // namespace APL
} // namespace LOFAR

#endif
