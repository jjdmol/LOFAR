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
#include "Beams.h"
#include <APL/RTCCommon/Timestamp.h>

#include <AMCBase/ConverterClient.h>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_TimerPort.h>

#include <set>
#include <map>
#include <list>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFTimerPort;
  using GCF::TM::GCFPortInterface;
  namespace BS {

extern	int		g_bf_gain;

class BeamServer : public GCFTask
{
private:
	// Class to maintain state on current beam
	// transaction (albeit Beamalloc or Beamfree)
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
	// The constructor of the BeamServer task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	BeamServer(string name, int argc, char** argv);
	virtual ~BeamServer();

	// Parse the commandline options
	void parseOptions(int argc, char** argv);

	// state methods

	// Method to clean up disconnected client ports.
	void undertaker();

	// Destroy all beams associated with port.
	void  destroyAllBeams(GCFPortInterface* port);

	// Create new beam and update administration
	Beam*   newBeam(BeamTransaction& 				bt, 
					GCFPortInterface* 				port,
					std::string 					nodeid, 
					std::string 					subarrayname,
					BS_Protocol::Beamlet2SubbandMap allocation);

	// Destroy beam of specified transaction.
	// @param bt the beamtransaction specifying the beam to destroy
	void deleteBeam(BeamTransaction& bt);

	// The initial state. This state is used to connect to the RSPDriver.
	GCFEvent::TResult con2rspdriver(GCFEvent& e, GCFPortInterface& p);

	// Try to connect to the CalServer
	GCFEvent::TResult con2calserver(GCFEvent& e, GCFPortInterface& p);

	// The enabled state. In this state the BeamServer can accept
	// client connections.
	GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface& p);

	// Cleanup state, when the rspdriver, the calserver or the
	// acceptor disconnects, then disconnect all clients and
	// return to the initial state.
	GCFEvent::TResult cleanup(GCFEvent& e, GCFPortInterface& p);

	// The beamalloc state. In this state the complete process
	// of allocating a beam, registering with the calibration
	// server, and getting the antenna positions is handled.
	GCFEvent::TResult beamalloc_state(GCFEvent& e, GCFPortInterface& p);

	// The beamfree state. In this state the BeamServer unsubscribes
	// with the calibration server for the specified beam.
	GCFEvent::TResult beamfree_state(GCFEvent& e, GCFPortInterface& p);

	void getAllHBADeltas(string filename);
	
	void getAllHBAElementDelays(string filename);
		
	// action methods

	// start allocation of a new beam
	bool beamalloc_start(BSBeamallocEvent& ba, GCFPortInterface& port);

	// free a beam
	bool beamfree_start(BSBeamfreeEvent& bf, GCFPortInterface& port);

	// Change the direction of a beam.
	bool beampointto_action(BSBeampointtoEvent& pt, GCFPortInterface& port);

	// Time to compute some more weights.
	// @param current_seconds Time in seconds since 1 Jan 1970
	void compute_weights(RTC::Timestamp time);

	// Send weights to the board.
	void send_weights(RTC::Timestamp time);

	// Time to compute the HBA delays
	// @param current_seconds Time in seconds since 1 Jan 1970
	void compute_HBAdelays(RTC::Timestamp time);

	// Send HBAdelays to the board.
	void send_HBAdelays(RTC::Timestamp	time);

	// Send subbands selection to the board.
	void send_sbselection();

	// defer event
	void defer(GCFEvent& e, GCFPortInterface& p);

	// recall event
	GCFEvent::TResult recall(GCFPortInterface& p);

private:
	// --- data members ---

	// Weight array
	blitz::Array<std::complex<double>,  3> m_weights;
	blitz::Array<std::complex<int16_t>, 3> m_weights16;

	// Relative positions of the elements of the HBA tile.
	blitz::Array<double, 2>		itsTileRelPos;	// [N_HBA_ELEMENTS,x|y] = [16,2]
		
	// Delay steps [0..31] of an element
	blitz::Array<double, 1>		itsElementDelays; // [N_HBA_DELAYS] = [32,1] 	

	// ports
	GCFTCPPort*       			 itsListener; 	 // list for clients on this port
	std::list<GCFPortInterface*> m_client_list;  // list of currently connected clients
	std::list<GCFPortInterface*> m_dead_clients; // list of discon. clients to be removed

	std::map<GCFPortInterface*, std::set<Beam*> >   m_client_beams; // mapping from client port to set of beams
	std::list<std::pair<char*, GCFPortInterface*> > m_deferred_queue; // deferred events

	BeamTransaction	m_bt; // current beam transaction

	GCFTCPPort* 			itsRSPDriver;			// connection to RSPDriver
	GCFTCPPort*				itsCalServer;  			// connection to CalServer
	GCFTimerPort*  			itsUpdateTimer;  		//
	bool     				m_beams_modified;		//
	bool					itsSetHBAEnabled;		//
	bool					itsSetWeightsEnabled;	//
	bool					itsSetSubbandsEnabled;	//
	int      				m_nrcus;				//
	Beams    				m_beams;				//
	AMC::ConverterClient 	m_converter;			//
	int32	 				m_instancenr;			//
	long					itsUpdateInterval;		//
	long					itsComputeInterval;		//
	long					itsHbaInterval;			//
};

  }; //# namespace BS
}; //# namespace LOFAR

#endif /* BEAMSERVER_H_ */
