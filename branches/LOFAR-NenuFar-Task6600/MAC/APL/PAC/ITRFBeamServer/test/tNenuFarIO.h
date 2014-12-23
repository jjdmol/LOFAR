//#  tNenuFarIO.h: Implements all IO with the tNenuFar Server.
//#
//#  Copyright (C) 2002-2014
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
//#  $Id: $

#ifndef BS_TNENUFARIO_H_
#define BS_TNENUFARIO_H_

// \file tNenuFarIO.h
// Implements all IO with the tNenuFar Server.

//# Never #include <config.h> or #include <lofar_config.h> in a header file!
//# Includes
#include <GCF/TM/GCF_Control.h>
#include "../src/NenuFarAdmin.h"

// Avoid 'using namespace' in headerfiles

namespace LOFAR {
  namespace BS {

using	MACIO::GCFEvent;
using	GCF::TM::GCFTimerPort;
using	GCF::TM::GCFTCPPort;
using	GCF::TM::GCFPortInterface;
using	GCF::TM::GCFTask;

// \addtogroup package
// @{

class tNenuFarIO : public GCFTask
{
public:
	explicit tNenuFarIO (NenuFarAdmin*	nnfAdmin);
	~tNenuFarIO();

private:
	// During the initial state all connections with the other programs are made.
	GCFEvent::TResult sunshine	   		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult noAnswer	   		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult testAbort	   		(GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult testMultipleBeams (GCFEvent& event, GCFPortInterface& port);
	GCFEvent::TResult testAbortAll		(GCFEvent& event, GCFPortInterface& port);

	//# --- Datamembers ---
	NenuFarAdmin*			itsAdmin;
	GCFTimerPort*			itsTimer;
};

//# --- Inline functions ---

// @}
  } // namespace BS
} // namespace LOFAR

#endif
