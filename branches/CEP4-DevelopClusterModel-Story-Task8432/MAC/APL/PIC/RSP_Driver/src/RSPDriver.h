//#  -*- mode: c++ -*-
//#
//#  RSPDriver.h: class definition for the Beam Server task.
//#
//#  Copyright (C) 2002-2004
//#  ASTRON (Netherlands Foundation for Research in Astronomy)
//#  P.O.Box 2, 7990 AA Dwingeloo, The Netherlands, softwaresupport@astron.nl
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

#ifndef RSPDRIVERTASK_H_
#define RSPDRIVERTASK_H_

#include <APL/RSP_Protocol/RSP_Protocol.ph>
#include <APL/RSP_Protocol/EPA_Protocol.ph>

#include <GCF/TM/GCF_Control.h>
#include <GCF/TM/GCF_ETHRawPort.h>
#include <GCF/TM/GCF_DevicePort.h>

#include "Scheduler.h"

#include <blitz/array.h>
#include <list>

#ifdef HAVE_SYS_TIMEPPS_H
#include <sys/timepps.h>
#endif

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFETHRawPort;
  namespace RSP {

class RSPDriver : public GCFTask
{
public:
	// The constructor of the RSPDriver task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	RSPDriver(string name);
	virtual ~RSPDriver();

	// Add all required synchronization actions.
	void addAllSyncActions();

	// Open or close all ports to boards.
	void openBoards();

	// @return true if ready to transition to the enabled
	// state.
	bool isEnabled();

	// Is this port connected to a board?
	bool isBoardPort(GCFPortInterface& port);

	// Fetch PPS
	// This method retries on EINTR or EAGAIN
	int fetchPPS();

	// Handle a F_DATAIN on the clock port.
	GCFEvent::TResult clock_tick(GCFPortInterface& port);

	// Delete the client ports on the m_dead_clients.
	void undertaker();

	// Try to load the PPS sychronisation delays for each BP
	void readPPSdelaySettings();

	// The initial state. This state is used to connect the client
	// and board ports. When they are both connected a transition
	// to the enabled state is made.
	GCFEvent::TResult initial(GCFEvent& event, GCFPortInterface &port);

	// The enabled state. In this state the task can receive
	// commands.
	GCFEvent::TResult enabled(GCFEvent& event, GCFPortInterface &port);

	/*@{*/
	// Handlers for the different events.
	void rsp_setweights(GCFEvent& event, GCFPortInterface &port);
	void rsp_getweights(GCFEvent& event, GCFPortInterface &port);

	void rsp_setsubbands  (GCFEvent& event, GCFPortInterface &port);
	void rsp_getsubbands  (GCFEvent& event, GCFPortInterface &port);
	void rsp_subsubbands  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubsubbands(GCFEvent& event, GCFPortInterface &port);

	void rsp_setrcu  (GCFEvent& event, GCFPortInterface &port);
	void rsp_getrcu  (GCFEvent& event, GCFPortInterface &port);
	void rsp_subrcu  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubrcu(GCFEvent& event, GCFPortInterface &port);

	void rsp_sethba  (GCFEvent& event, GCFPortInterface &port);
	void rsp_gethba  (GCFEvent& event, GCFPortInterface &port);
	void rsp_readhba (GCFEvent& event, GCFPortInterface &port);
	void rsp_subhba  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubhba(GCFEvent& event, GCFPortInterface &port);

	void rsp_setrsu  (GCFEvent& event, GCFPortInterface &port);

	void rsp_setwg(GCFEvent& event, GCFPortInterface &port);
	void rsp_getwg(GCFEvent& event, GCFPortInterface &port);

	void rsp_substatus  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubstatus(GCFEvent& event, GCFPortInterface &port);
	void rsp_getstatus  (GCFEvent& event, GCFPortInterface &port);

	void rsp_substats  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubstats(GCFEvent& event, GCFPortInterface &port);
	void rsp_getstats  (GCFEvent& event, GCFPortInterface &port);

	void rsp_subxcstats  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubxcstats(GCFEvent& event, GCFPortInterface &port);
	void rsp_getxcstats  (GCFEvent& event, GCFPortInterface &port);

	void rsp_getversions(GCFEvent& event, GCFPortInterface &port);

	void rsp_getconfig(GCFEvent& event, GCFPortInterface &port);

	void rsp_setclock(GCFEvent& event, GCFPortInterface &port);
	void rsp_getclock(GCFEvent& event, GCFPortInterface &port);
	void rsp_subclock(GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubclock(GCFEvent& event, GCFPortInterface &port);

	void rsp_subtdstatus  (GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubtdstatus(GCFEvent& event, GCFPortInterface &port);
	void rsp_gettdstatus  (GCFEvent& event, GCFPortInterface &port);

	void rsp_getregisterstate(GCFEvent& event, GCFPortInterface &port);
	void rsp_subregisterstate(GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubregisterstate(GCFEvent& event, GCFPortInterface &port);

	void rsp_settbb(GCFEvent& event, GCFPortInterface &port);
	void rsp_gettbb(GCFEvent& event, GCFPortInterface &port);

	void rsp_setbypass(GCFEvent& event, GCFPortInterface &port);
	void rsp_getbypass(GCFEvent& event, GCFPortInterface &port);

	void rsp_getspustatus(GCFEvent& event, GCFPortInterface &port);

	void rsp_setRawBlock(GCFEvent& event, GCFPortInterface &port);
	void rsp_getRawBlock(GCFEvent& event, GCFPortInterface &port);

	void rsp_setSplitter(GCFEvent& event, GCFPortInterface &port);
	void rsp_getSplitter(GCFEvent& event, GCFPortInterface &port);
	void rsp_subSplitter(GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubSplitter(GCFEvent& event, GCFPortInterface &port);
	
	void rsp_latencys(GCFEvent& event, GCFPortInterface &port);
	
	void rsp_setDatastream(GCFEvent& event, GCFPortInterface &port);
	void rsp_getDatastream(GCFEvent& event, GCFPortInterface &port);
	
	void rsp_setswapxy(GCFEvent& event, GCFPortInterface& port);
	void rsp_getswapxy(GCFEvent& event, GCFPortInterface& port);
	
	void rsp_setBitMode(GCFEvent& event, GCFPortInterface &port);
	void rsp_getBitMode(GCFEvent& event, GCFPortInterface &port);
	void rsp_subBitMode(GCFEvent& event, GCFPortInterface &port);
	void rsp_unsubBitMode(GCFEvent& event, GCFPortInterface &port);
	
    void rsp_setSDOMode(GCFEvent& event, GCFPortInterface &port);
	void rsp_getSDOMode(GCFEvent& event, GCFPortInterface &port);
    void rsp_setSDO(GCFEvent& event, GCFPortInterface &port);
	void rsp_getSDO(GCFEvent& event, GCFPortInterface &port);
    /*@}*/

private:
	// define some constants
	// Synchronisation mode
	static const int32	SYNC_SOFTWARE     = 1; 	// generated by software timer
	static const int32	SYNC_FAST         = 2; 	// as fast as possible
	static const int32	SYNC_PPS          = 3; 	// PPS on /dev/pps0
	static const int32	SYNC_NO_PPS_USAGE = 4; 	// PPS is not used, soft wait for second transition
	int32		itsSyncMode;					// read from file
	uint32		itsLastSecond;					// last time (no)PPS timer elapsed

	//# ----- data members -----

	// ports
	GCFTCPPort                   m_acceptor;     // listen for clients on this port
	GCFETHRawPort*               m_boardPorts;   // array of ports, one for each RSP board
	std::list<GCFPortInterface*> m_client_list;  // list of clients
	std::list<GCFPortInterface*> m_dead_clients; // list of clients to cleanup

	Scheduler				m_scheduler;
	int						m_update_counter; 	// nr of updates completed in one second
	int						m_n_updates;		// number of completed updates
	int						m_elapsed;			// elapsed number of seconds

	blitz::Array<int, 1>	itsPPSsyncDelays;	// one delay for each AP

#ifdef HAVE_SYS_TIMEPPS_H
	int						m_ppsfd;			// file descriptor for PPS device
	pps_handle_t			m_ppshandle;		// handle to PPS API interface
	pps_info_t				m_ppsinfo;			// most recent ppsinfo
#endif
	int						itsPPSdelay;		// nr of usec to wait after PPS was received or simulated.
};

  } // namespace RSP
} // namespace LOFAR

#endif /* RSPDRIVERTASK_H_ */
