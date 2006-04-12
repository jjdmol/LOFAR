//#  BeamServer.h: class definition for the Beam Server task.
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

#ifndef BEAMSERVER_H_
#define BEAMSERVER_H_

#include <APL/CAL_Protocol/SpectralWindow.h>
#include <APL/BS_Protocol/BS_Protocol.ph>
#include "Beam.h"
#include "Beamlet.h"
#include <APL/RTCCommon/Timestamp.h>

#include <AMCBase/AMCClient/ConverterClient.h>

#include <GCF/TM/GCF_Control.h>

#include <set>
#include <map>
#include <list>

namespace LOFAR {
  namespace BS {

    class BeamServer : public GCFTask
      {
      private:

	/**
	 * Class to maintain state on current beam
	 * transaction (albeit Beamalloc or Beamfree)
	 */
	class BeamTransaction
	  {
	  public:
	    BeamTransaction() : m_port(0), m_beam(0) /*, event(0), calhandle(0)*/ {}

	    void set(GCFPortInterface* port, Beam* beam) { m_port = port; m_beam = beam; }
	    inline void reset() { set(0,0); }

	    GCFPortInterface* getPort() const { return m_port; }
	    Beam*             getBeam() const { return m_beam; }

	  private:
	    // Port on which the transaction is taking place.
	    GCFPortInterface* m_port;

	    // Beam that is the subject of the transaction
	    Beam* m_beam;
	  };

      public:
	/**
	 * The constructor of the BeamServer task.
	 * @param name The name of the task. The name is used for looking
	 * up connection establishment information using the GTMNameService and
	 * GTMTopologyService classes.
	 */
	BeamServer(string name);
	virtual ~BeamServer();

	// state methods

	/**
	 * Method to clean up disconnected client ports.
	 */
	void undertaker();

	/**
	 * Destroy all beams associated with port.
	 */
	void  destroyAllBeams(GCFPortInterface* port);

	/**
	 * Create new beam and update administration
	 */
	Beam* newBeam(BeamTransaction& bt, GCFPortInterface* port,
		      std::string nodeid, std::string subarrayname,
		      BS_Protocol::Beamlet2SubbandMap allocation);

	/**
	 * Destroy beam of specified transaction.
	 * @param bt the beamtransaction specifying the beam to destroy
	 */
	void deleteBeam(BeamTransaction& bt);

	/**
	 * @return true if ready to transition to the enabled
	 * state.
	 */
	bool isEnabled();

	/**
	 * The initial state. This state is used to connect to
	 * the RSPDriver and the CalibrationServer. When both 
	 * are connected a transition to the enabled state is made.
	 */
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface& p);

	/**
	 * The enabled state. In this state the BeamServer can accept
	 * client connections.
	 */
	GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface& p);

	/**
	 * Cleanup state, when the rspdriver, the calserver or the
	 * acceptor disconnects, then disconnect all clients and
	 * return to the initial state.
	 */
	GCFEvent::TResult cleanup(GCFEvent& e, GCFPortInterface& p);

	/**
	 * The beamalloc state. In this state the complete process
	 * of allocating a beam, registering with the calibration
	 * server, and getting the antenna positions is handled.
	 */
	GCFEvent::TResult beamalloc_state(GCFEvent& e, GCFPortInterface& p);

	/**
	 * The beamfree state. In this state the BeamServer unsubscribes
	 * with the calibration server for the specified beam.
	 */
	GCFEvent::TResult beamfree_state(GCFEvent& e, GCFPortInterface& p);

	// action methods

	/**
	 * start allocation of a new beam
	 */
	bool beamalloc_start(BSBeamallocEvent& ba,
			     GCFPortInterface& port);

	/**
	 * free a beam
	 */
	bool beamfree_start(BSBeamfreeEvent& bf,
			    GCFPortInterface& port);

	/**
	 * Change the direction of a beam.
	 */
	bool beampointto_action(BSBeampointtoEvent& pt,
				GCFPortInterface& port);

	/**
	 * Time to compute some more weights.
	 * @param current_seconds Time in seconds since 1 Jan 1970
	 */
	void compute_weights(RTC::Timestamp time);

	/**
	 * Send weights to the board.
	 */
	void send_weights(RTC::Timestamp time);

	/**
	 * Send subbands selection to the board.
	 */
	void send_sbselection();

	/**
	 * defer event
	 */
	void defer(GCFEvent& e, GCFPortInterface& p);

	/**
	 * recall event
	 */
	GCFEvent::TResult recall(GCFPortInterface& p);

      private:
	// member variables

	/**
	 * Current subband selection
	 */
	BS_Protocol::Beamlet2SubbandMap m_sbsel;

	/**
	 * Receptor positions in 3 dimensions (x, y and z)
	 */
	blitz::Array<double, 3> m_pos;

	/**
	 * Weight array
	 */
	blitz::Array<std::complex<double>,  3> m_weights;
	blitz::Array<std::complex<int16_t>, 3> m_weights16;

      private:
	// ack events
	//BS_Protocol::BeamallocackEvent m_beamallocack;
	//BS_Protocol::BeamfreeackEvent  m_beamfreeack;

	// ports
	GCFTCPPort       m_acceptor; // list for clients on this port
	std::list<GCFPortInterface*> m_client_list; // list of currently connected clients
	std::list<GCFPortInterface*> m_dead_clients; // list of disconnected clients to be removed

	std::map<GCFPortInterface*, std::set<Beam*> >  m_client_beams; // mapping from client port to set of beams
	std::map<GCFPortInterface*, std::list<char*> > m_deferred_queue; // deferred events

	BeamTransaction m_bt; // current beam transaction

	GCFPort  m_rspdriver;
	GCFPort  m_calserver;  
	bool     m_beams_modified;

	int      m_nrcus;

	int32    m_sampling_frequency;
	int32    m_nyquist_zone;    
	Beams    m_beams;

	AMC::ConverterClient m_converter;
      };
  };
};
     
#endif /* BEAMSERVER_H_ */
