//  disabledRCUlist.h: Small utility to gat the list of disable rcus.
//
//  Copyright (C) 2011
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
//  $Id: tDPservice.h 10538 2007-10-03 15:04:43Z overeem $
//

#ifndef _RTDBCOMMON_DISABLEDRCULIST_H
#define _RTDBCOMMON_DISABLEDRCULIST_H

#include <GCF/TM/GCF_Control.h>
#include <GCF/RTDB/DPservice.h>
#include <Common/LofarConstants.h>

namespace LOFAR {
 namespace APL {
  namespace RTDBCommon {

class mainTask : public GCF::TM::GCFTask
{
public:
	mainTask (const string& name, int rcuMode);
	virtual ~mainTask();

	GCFEvent::TResult takeSubscription (GCFEvent& event, GCF::TM::GCFPortInterface& port);
	void 			  _handleQueryEvent(GCFEvent& event);

private:
	GCF::TM::GCFTimerPort*		itsTimerPort;
	GCF::RTDB::DPservice*		itsDPservice;
	bitset<MAX_ANTENNAS*2>		itsRCUmask;
	int							itsRCUmode;
};

  } // namespace RTDBCommon
 } // namespace APL
} // namespace LOFAR

#endif
