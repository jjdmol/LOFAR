//#  ABSBeamServerTask.h: class definition for the Beam Server task.
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

#ifndef ABSBEAMSERVERTASK_H_
#define ABSBEAMSERVERTASK_H_

#include "ABSSpectralWindow.h"
#include "ABS_Protocol.ph"

#include <GCF/GCF_Control.h>
#include <GCF/GCF_ETHRawPort.h>

#include <set>
#include <map>

namespace ABS
{
  class Beam;
  class Beamlet;
  class Subband;

    class BeamServerTask : public GCFTask
    {
    public:
	/**
	 * The constructor of the BeamServerTask task.
	 * @param name The name of the task. The name is used for looking
	 * up connection establishment information using the GTMNameService and
	 * GTMTopologyService classes.
	 */
	BeamServerTask(string name);
	virtual ~BeamServerTask();

	// state methods

	/**
	 * The initial state. This state is used to connect the client
	 * and board ports. When they are both connected a transition
	 * to the enabled state is made.
	 */
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &p);

	/**
	 * The enabled state. In this state the task can receive
	 * commands.
	 */
	GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &p);

	// action methods
	
	/**
	 * allocate a new beam
	 */
	void beamalloc_action(ABSBeamallocEvent* ba,
			      GCFPortInterface& port);

	/**
	 * free a beam
	 */
	void beamfree_action(ABSBeamfreeEvent* bf,
			     GCFPortInterface& port);

	/**
	 * Change the direction of a beam.
	 */
	void beampointto_action(ABSBeampointtoEvent* pt,
				GCFPortInterface& port);

	/**
	 * Start a new compute cycle.
	 */
	void compute_timeout();

	/**
	 * Calculate beam former weights.
	 */
	void calc_weigths();

	/**
	 * Determine the new subband selection after a beam
	 * has been allocated or freed.
	 */
	void update_sbselection();

    private:
	// member variables

	/**
	 * Set of currently allocated beams by index.
	 */
	std::set<int> m_beams;

	/**
	 * List of configured spectral windowds.
	 */
	std::map<int, SpectralWindow*> m_spws;

    private:
	// ports
	GCFPort       client;
	GCFETHRawPort board;

    };

};
     
#endif /* ABSBEAMSERVERTASK_H_ */
