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

#include <Common/lofar_bitset.h>
#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <Common/lofar_set.h>
#include <Common/lofar_string.h>
#include <GCF/TM/GCF_Control.h>
#include <APL/IBS_Protocol/IBS_Protocol.ph>
#include <CASATools/CasaConverter.h>
#include "DigitalBeam.h"
#include "AnaBeamMgr.h"
#include "StatCal.h"

namespace LOFAR {
  using namespace CASATools;
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
	~BeamServer();

private:
	// Method to clean up disconnected client ports.
	void undertaker();

	// Destroy all beams associated with port.
	void  destroyAllBeams(GCFPortInterface* port);

	// Create new beam and update administration
	DigitalBeam*	checkBeam(GCFPortInterface* 					port,
					string 								name, 
					string 								subarrayname, 
					IBS_Protocol::Beamlet2SubbandMap	allocation,
					bitset<LOFAR::MAX_RCUS>				rcumask,
					uint								ringNr,
					uint								rcuMode,
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

	// Take a subscription on the bitmode
	GCFEvent::TResult subscribeBitmode(GCFEvent& event, GCFPortInterface& port);

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

	// action methods

	// start allocation of a new beam
	bool beamalloc_start(IBSBeamallocEvent& ba, GCFPortInterface& port);

	// free a beam
	bool beamfree_start(IBSBeamfreeEvent& bf, GCFPortInterface& port);

	// Change the direction of a beam.
	int beampointto_action(IBSPointtoEvent& pt, GCFPortInterface& port);

	// Time to compute some more weights.
	// @param current_seconds Time in seconds since 1 Jan 1970
	void compute_weights(RTC::Timestamp time);

	// Send weights to the board.
	void send_weights(RTC::Timestamp time);

	// Send subbands selection to the board.
	void send_sbselection();

	// Calculate best starttime for analoge beam
	int	_idealStartTime (int now, int t1, int d1, int t2, int d2, int p2) const;

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
	void _registerBeamRCUs   (const DigitalBeam&	beam);
	void _unregisterBeamRCUs (const DigitalBeam&	beam);
	void _logBeamAdministration();

	// RCU calibration
	std::complex<double>	_getCalFactor(uint rcuMode, uint rcu, uint subbandNr);
	void 					_loadCalTable(uint rcuMode, uint nrRSPBoards);

	// ### data members ###

	// 'constant' containing the current number of bits each datasample has.
	// This value determines how many beamlets a RSPBoard produces (and how large some of
	// our arrays become). The function maxBeamletsPerRSP in Common/LofarBitModeInfo.h calculates
	// the maxBeamlets 'constant' we are used to work with before bitsperSample was a variable value.
	int		itsCurrentBitsPerSample;
	int		itsCurrentMaxBeamlets;

	// BeamletAllocation
	typedef struct BeamletAllocation {
		int						subbandNr;
		std::complex<double>	scaling;
		BeamletAllocation(int sb, std::complex<double> scale) : subbandNr(sb), scaling(scale) {};
	} BeamletAlloc_t;
	vector<BeamletAlloc_t>		itsBeamletAllocation;

	// Weights array [MAX_RCUS, MAX_BEAMLETS]
	blitz::Array<std::complex<double>,  3> itsWeights;
	blitz::Array<std::complex<int16_t>, 3> itsWeights16;

	// RCU Allocations in the AntennaArrays. Remember that each RCU can participate 
	// in more than one beam.
	bitset<MAX_RCUS>			itsLBAallocation;	// which RCUs are used for LBA
	bitset<MAX_RCUS>			itsHBAallocation;	// which RCUs are used for HBA
	vector<uint>				itsLBArcus;			// counter: in how many beams the RCU participates
	vector<uint>				itsHBArcus;			// counter: in how many beams the RCU participates
	uint						itsNrLBAbeams;
	uint						itsNrHBAbeams;
	long						itsLastHBACalculationTime;

	// ports
	GCFTCPPort*					itsListener;	// list for clients on this port
	list<GCFPortInterface*> 	itsClientList;	// list of currently connected clients
	list<GCFPortInterface*> 	itsDeadClients;	// list of discon. clients to be removed

	map<GCFPortInterface*, set<DigitalBeam*> >   itsClientBeams; // mapping from client port to set of beams

	BeamTransaction				itsBeamTransaction; 	// current beam transaction

	CasaConverter*				itsJ2000Converter;		// casacore based converter to J2000
	GCFTCPPort* 				itsRSPDriver;			// connection to RSPDriver
	GCFTCPPort*					itsCalServer;  			// connection to CalServer
	GCFTimerPort*  				itsDigHeartbeat;	  	// heartbeat for digital beamformer weights
	GCFTimerPort*  				itsAnaHeartbeat;  		// heartbeat for analogue beamformer delays
	GCFTimerPort*  				itsConnectTimer;	  	// General (re)connect timer
	bool     					itsBeamsModified;		//
	bool						itsSplitterOn;			// state of the ringsplitter
	map<string, DigitalBeam*> 	itsBeamPool;			//
	AnaBeamMgr*					itsAnaBeamMgr;			// for managing the analogue beams

	StatCal*					itsCalTableMode1;		// table for mode 1 and 2
	StatCal*					itsCalTableMode3;		// table for mode 3 and 4
	StatCal*					itsCalTableMode5;		// table for mode 5
	StatCal*					itsCalTableMode6;		// table for mode 6
	StatCal*					itsCalTableMode7;		// table for mode 7
	
	// constants
	uint   	itsMaxRCUs;				//
	uint   	itsMaxRSPboards;		//
	bool	itsSetHBAEnabled;		//
	bool	itsSetWeightsEnabled;	//
	bool	itsSetSubbandsEnabled;	//
	bool	itsStaticCalEnabled;	//
	long	itsUpdateInterval;		//
	long	itsComputeInterval;		//
	long	itsHBAUpdateInterval;	//

	long	itsTestSingleShotTimestamp;	// FOR TESTING PURPOSES
};

  }; //# namespace BS
}; //# namespace LOFAR

#endif /* BEAMSERVER_H_ */
