//#  ABSEPAStub.h: class definition for the Virtual Telescope test stub.
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

#ifndef ABSEPASTUB_H_
#define ABSEPASTUB_H_

#include "ABS_Protocol.ph"

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>

namespace ABS
{
    class EPAStub : public GCFTask
    {
    public:
	/**
	 * The constructor of the EPAStub task.
	 * @param name The name of the task. The name is used for looking
	 * up connection establishment information using the GTMNameService and
	 * GTMTopologyService classes.
	 */
	EPAStub(string name);
	virtual ~EPAStub();

	// state methods

	/**
	 * The initial state. In this state the beam_server port
	 * is opened.
	 */
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

	/**
	 * The enabled state. This state is reached when the
	 * beam_server port is connected.
	 */
	GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &p);

    private:
	// member variables

    private:
	// ports
	GCFPort       beam_server;

    };

};
     
#endif /* ABSAVTSTUB_H_ */
