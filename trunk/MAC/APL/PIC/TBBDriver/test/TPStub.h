//#  -*- mode: c++ -*-
//#
//#  TPStub.h: class definition for the TP stub task
//#
//#  Copyright (C) 2002-2004
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

#ifndef TPSTUB_H_
#define TPSTUB_H_

#include <Suite/test.h>
#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

#include "TP_Protocol.ph"

namespace LOFAR {
	using GCF::TM::GCFTask;
	using GCF::TM::GCFETHRawPort;
	using GCF::TM::GCFPortInterface;
	namespace TBB_Test {

class TPStub : public GCFTask, public Test
{
public:
	/**
	* The constructor of the TPStub task.
	* @param name The name of the task. The name is used for looking
	* up connection establishment information using the GTMNameService and
	* GTMTopologyService classes.
	*/
	
	TPStub(string name);
	virtual ~TPStub();

	// state methods

	/**
	* The initial and final state.
	*/
	/*@{*/
	GCFEvent::TResult initial(GCFEvent &event, GCFPortInterface &port);
	GCFEvent::TResult final(GCFEvent &event, GCFPortInterface &port);
	/*@}*/

	/**
	* The stub states.
	*/
	GCFEvent::TResult connected(GCFEvent &event, GCFPortInterface &port);


	/**
	* Run the tests.
	*/
	void run();

private:
	// member variables

private:
	// ports
	GCFETHRawPort itsServer;
};

	}
}

#endif /* TPSTUB_H_ */
