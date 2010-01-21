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

#include <Common/lofar_string.h>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include "J2000Converter.h"
#include "DigitalBeam.h"

#include <GCF/TM/GCF_Control.h>
//#include <GCF/TM/GCF_TimerPort.h>
#include <APL/APLCommon/AntennaSets.h>
#include <APL/APLCommon/AntennaPos.h>

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
	class BeamTransaction {
	public:
		BeamTransaction() : m_port(0), m_beam(0), itsAllocDone(false) {}

		void set(GCFPortInterface* port, DigitalBeam* beam) { m_port = port; m_beam = beam; }
		inline void reset() 			{ set(0,0); itsAllocDone = false; }
		inline void allocationDone() 	{ itsAllocDone = true; }
		inline bool isAllocationDone()	{ return(itsAllocDone); }

		GCFPortInterface* getPort() const { return m_port; }
		DigitalBeam*      getBeam() const { return m_beam; }

	private:
		// Port on which the transaction is taking place.
		GCFPortInterface* m_port;

		// Beam that is the subject of the transaction
		DigitalBeam* m_beam;

		bool	itsAllocDone;
	};

public:
	// The constructor of the BeamServer task.
	// @param name The name of the task.
	explicit BeamServer(const string& name, long timestamp = 0);
	virtual ~BeamServer();

	// Method to clean up disconnected client ports.
	void undertaker();

	// Destroy all beams associated with port.
	void  destroyAllBeams(GCFPortInterface* port);

	// Create new beam and update administration
	DigitalBeam*	checkBeam(GCFPortInterface* 					port,
					string 								name, 
					string 								subarrayname, 
					IBS_Protocol::Beamlet2SubbandMap	allocation,
					LOFAR::bitset<LOFAR::MAX_RCUS>		rcumask,
					int									ringNr,
					int*								beamError);

	// Destroy beam of specified transaction.
	// @param bt the beamtransaction specifying the beam to destroy
	void deleteTransactionBeam();

	// The initial state. This state is used to connect to the RSPDriver.
	GCFEvent::TResult con2rspdriver(GCFEvent& e, GCFPortInterface& p);

	// Ask the RSPdriver how much hardware is available
	GCFEvent::TResult askConfiguration(GCFEvent& e, GCFPortInterface& p);

	// Take a subscription on the splitter state.
	GCFEvent::TResult subscribeSplitter(GCFEvent& e, GCFPortInterface& p);

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
	bool beamalloc_start(IBSBeamallocEvent& ba, GCFPortInterface& port);

	// free a beam
	bool beamfree_start(IBSBeamfreeEvent& bf, GCFPortInterface& port);

	// Change the direction of a beam.
	bool beampointto_action(IBSBeampointtoEvent& pt, GCFPortInterface& port);

	// Time to compute some more weights.
	// @param current_seconds Time in seconds since 1 Jan 1970
	bool compute_weights(RTC::Timestamp time);

	// Send weights to the board.
	void send_weights(RTC::Timestamp time);

	// Time to compute the HBA delays
	// @param current_seconds Time in seconds since 1 Jan 1970
	void compute_HBAdelays(RTC::Timestamp time);

	// Send HBAdelays to the board.
	void send_HBAdelays(RTC::Timestamp	time);

	// Send subbands selection to the board.
	void send_sbselection();

	// (re)create the beampool. Normally done after the splitter state changed.
	void _createBeamPool();

	bool _checkBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
					    uint								ringNr);
	void _allocBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
					    uint								ringNr);
	void _scaleBeamlets(IBS_Protocol::Beamlet2SubbandMap&	allocation,
					    uint								ringNr,
						const CAL::SpectralWindow&			spw);
	void _releaseBeamlets (IBS_Protocol::Beamlet2SubbandMap&	allocation,
						   uint								ringNr);

	vector<double>  blitz2vector(const blitz::Array<double,1>&    anBA) const;

	// RCU administration
	void _registerBeam   (const DigitalBeam&	beam);
	void _unregisterBeam (const DigitalBeam&	beam);
	void _logBeamAdministration();

private:
	// --- data members ---

	// Weights array [MAX_RCUS, MAX_BEAMLETS]
	blitz::Array<std::complex<double>,  2> itsWeights;
	blitz::Array<std::complex<int16_t>, 2> itsWeights16;

	// Antenna positions in ITRF and subsets of the antennafields.
	APLCommon::AntennaPos*		itsAntennaPos;			// positions of LBA and HBA antennas
	APLCommon::AntennaSets*		itsAntennaSets;			// all possible antennaSets

	// Administration for the analogue HBA beamforming
	// Relative positions of the elements of the HBA tile.
	blitz::Array<double, 2>		itsTileRelPos;	// [N_HBA_ELEMENTS,x|y] = [16,2]
	// Delay steps [0..31] of an element
	blitz::Array<double, 1>		itsDelaySteps; // [N_HBA_DELAYS] = [32,1] 	
	// The caluclated delays for each element for each HBA tile.
	blitz::Array<uint8, 2> 		itsHBAdelays;		// [rcus, N_HBA_ELEM_PER_TILE]

	// BeamletAllocation
	typedef struct BeamletAllocation {
		int						subbandNr;
		std::complex<double>	scaling;
		BeamletAllocation(int sb, std::complex<double> scale) : subbandNr(sb), scaling(scale) {};
	} BeamletAlloc_t;
	vector<BeamletAlloc_t>		itsBeamletAllocation;

	// RCU Allocations in the AntennaArrays. Remember that each RCU can participate 
	// in more than one beam.
	bitset<MAX_RCUS>			itsLBAallocation;
	bitset<MAX_RCUS>			itsHBAallocation;
	vector<uint>				itsLBArcus;
	vector<uint>				itsHBArcus;
	uint						itsNrLBAbeams;
	uint						itsNrHBAbeams;

	// ports
	GCFTCPPort*					itsListener;	// list for clients on this port
	list<GCFPortInterface*> 	itsClientList;	// list of currently connected clients
	list<GCFPortInterface*> 	itsDeadClients;	// list of discon. clients to be removed

	map<GCFPortInterface*, set<DigitalBeam*> >   itsClientBeams; // mapping from client port to set of beams

	BeamTransaction				itsBeamTransaction; 	// current beam transaction

	J2000Converter				itsJ2000Converter;		// casacore based converter to J2000
	GCFTCPPort* 				itsRSPDriver;			// connection to RSPDriver
	GCFTCPPort*					itsCalServer;  			// connection to CalServer
	GCFTimerPort*  				itsHeartbeatTimer;  	//
	GCFTimerPort*  				itsTimerPort;		  	// General purpose timer
	bool     					itsBeamsModified;		//
	bool						itsSplitterOn;			// state of the ringsplitter
	map<string, DigitalBeam*> 	itsBeamPool;			//
	
	// constants
	int    	itsMaxRCUs;				//
	bool	itsSetHBAEnabled;		//
	bool	itsSetWeightsEnabled;	//
	bool	itsSetSubbandsEnabled;	//
	long	itsUpdateInterval;		//
	long	itsComputeInterval;		//
	long	itsHbaInterval;			//

	long	itsTestSingleShotTimestamp;	// FOR TESTING PURPOSES
};

  }; //# namespace BS
}; //# namespace LOFAR

#endif /* BEAMSERVER_H_ */
