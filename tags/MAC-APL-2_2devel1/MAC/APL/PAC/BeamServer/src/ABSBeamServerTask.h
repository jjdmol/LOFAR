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
#include "ABSBeamlet.h"

#include <GCF/TM/GCF_Control.h>

#include <set>
#include <map>
#include <list>

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
	 * Method to clean up disconnected client ports.
	 */
	void collect_garbage();

	/**
	 * @return true if ready to transition to the enabled
	 * state.
	 */
	bool isEnabled();

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

	/**
	 * The wait4ack state. This state is entered when a request has
	 * been sent to the RSP driver and the BeamServer is waiting
	 * for the acknowledgement. After receiving the acknowledgement
	 * the statemachine returns to the 'enabled' state.
	 */
	GCFEvent::TResult wait4ack(GCFEvent& e, GCFPortInterface &p);

	/**
	 * This handler handles all events that should be handled
	 * irrespective of the state ('enabled' or 'waiting').
	 */
	GCFEvent::TResult handle_abs_request(GCFEvent& e, GCFPortInterface &p);

	// action methods

	/**
	 * allocate a new beam
	 */
	void beamalloc_action(ABSBeamallocEvent& ba,
			      GCFPortInterface& port);

	/**
	 * free a beam
	 */
	void beamfree_action(ABSBeamfreeEvent& bf,
			     GCFPortInterface& port);

	/**
	 * Change the direction of a beam.
	 */
	void beampointto_action(ABSBeampointtoEvent& pt,
				GCFPortInterface& port);

	/**
	 * Enable or change setting of the waveform generator.
	 * Enabling the waveform generator disables the ADC input.
	 */
	void wgsettings_action(ABSWgsettingsEvent& we,
			       GCFPortInterface& port);
	void wgenable_action();

	/**
	 * Disable the waveform generator.
	 * This enables the ADC input.
	 */
	void wgdisable_action();
			      
	/**
	 * Time to compute some more weights.
	 * @param current_seconds Time in seconds since 1 Jan 1970
	 */
	void compute_weights(long current_seconds);

	/**
	 * Send weights to the board.
	 */
	void send_weights();

	/**
	 * Determine the new subband selection after a beam
	 * has been allocated or freed.
	 */
	void update_sbselection();

	/**
	 * Send subbands selection to the board.
	 */
	void send_sbselection();

#if 0
	/**
	 * Defer an event from m_client to the save queue
	 * for later processing.
	 */
	void saveq_defer(GCFEvent& e);

	/**
	 * Recall the least recently deferred event.
	 */
	GCFEvent* saveq_recall();

	/**
	 * Pop and delete the event.
	 */
	void saveq_pop();

	/**
	 * Clear the saveq.
	 */
	void saveq_clear();
#endif

    private:
	// member variables

	/**
	 * List of configured spectral windowds.
	 */
	std::map<int, SpectralWindow*> m_spws;

	/**
	 * Current subband selection
	 */
	std::map<int, int> m_sbsel;

	/**
	 * Set of curently allocated beams.
	 */
	std::set<Beam*> m_beams;

	/**
	 * Current WG settings.
	 */
	struct {
	    double         frequency;
	    unsigned short amplitude;
	    bool           enabled;
	} m_wgsetting;

	/**
	 * Receptor positions
	 */
	blitz::Array<W_TYPE, 3> m_pos;

	/**
	 * Weight array
	 */
	blitz::Array<std::complex<W_TYPE>,  4> m_weights;
	blitz::Array<std::complex<int16_t>, 4> m_weights16;

    private:
	// ports
	GCFTCPPort       m_acceptor; // list for clients on this port
	std::list<GCFPortInterface*> m_client_list; // list of currently connected clients
	std::list<GCFPortInterface*> m_garbage_list; // list of disconnected clients to be removed

	std::map<GCFPortInterface*, std::set<int> > m_client_beams; // mapping from client port to set of beam handles

	GCFPort          m_rspdriver;
	std::list<char*> m_saveq;
	bool             m_subbands_modified;
    };

};
     
#endif /* ABSBEAMSERVERTASK_H_ */
