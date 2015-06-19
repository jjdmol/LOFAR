//
//  tServer.h: Definition of the Echo task class.
//
//  Copyright (C) 2009
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

#ifndef _TSERVER_H_
#define _TSERVER_H_

#include <GCF/TM/GCF_Control.h>

namespace LOFAR {
 namespace GCF {
  namespace TM {

class tServer : public GCFTask
{
public:
	tServer (string name, uint	startupDelay);

	GCFEvent::TResult initial  (GCFEvent& e, GCFPortInterface& p);
	GCFEvent::TResult connected(GCFEvent& e, GCFPortInterface& p);

private:
	GCFTCPPort* 	itsListener;
	GCFTimerPort*	itsTimerPort;
	uint			itsStartupDelay;
};

  } // namespace TM
 } // namespace GCF
} // namespace LOFAR
#endif
