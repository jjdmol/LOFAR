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
#include "ABSSubbandStats.h"

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
	 * Maximum number of beamlets produced by
	 * the server. The number of beamlets of
	 * all beams together can not be larger
	 * than this value. This is always less
	 * than or equal to N_SUBBANDS
	 */
	static const int N_BEAMLETS = 128;

	/**
	 * Maximum number of input subbands of the
	 * system. Each spectral window should have
	 * n_subbands <= N_SUBBANDS. This is always
	 * greater than or equal to N_BEAMLETS.
	 */
	static const int N_SUBBANDS = 128;

	/**
	 * The number of antenna elements, an
	 * element can have one or two polarizations.
	 */
	static int const N_ELEMENTS      = 1;

	/**
	 * Number of polarizations for each element.
	 */
	static int const N_POLARIZATIONS = 2;

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
	 * Enable or change setting of the waveform generator.
	 * Enabling the waveform generator disables the ADC input.
	 */
	void wgsettings_action(ABSWgsettingsEvent* we,
			       GCFPortInterface& port);
	void wgenable_action();
	void sbselect();

	/**
	 * Disable the waveform generator.
	 * This enables the ADC input.
	 */
	void wgdisable_action();
			      
	/**
	 * Time to compute some more weights.
	 * @param current_seconds Time in seconds since 1 Jan 1970
	 */
	void compute_timeout_action(long current_seconds);

	/**
	 * Send weights to the board.
	 */
	void send_weights(int period);

	/**
	 * Determine the new subband selection after a beam
	 * has been allocated or freed.
	 */
	void update_sbselection();

	/**
	 * Send subbands selection to the board.
	 */
	void send_sbselection();

    private:
	// member variables

	/**
	 * List of configured spectral windowds.
	 */
	std::map<int, SpectralWindow*> m_spws;

	/**
	 * Set of currently allocated beams by index.
	 */
	std::set<Beam*> m_beams;

	/**
	 * Current subband selection
	 */
	std::map<int, int> m_sbsel;

	/**
	 * Current WG settings.
	 */
	struct {
	    double        frequency;
	    unsigned char amplitude;
	    unsigned char sample_period;
	    bool          enabled;
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

	/**
	 * Subband statistics object to aggregate statistics
	 * and to export the statistics as a property.
	 */
	SubbandStats m_stats;

    private:
	// ports
	GCFPort       client;
	GCFETHRawPort board;

    };

};
     
#endif /* ABSBEAMSERVERTASK_H_ */
