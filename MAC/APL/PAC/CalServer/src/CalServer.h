//#  -*- mode: c++ -*-
//#  CalServer.h: class definition for the CalServe task.
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

#ifndef CALSERVER_H_
#define CALSERVER_H_

#include <Common/lofar_list.h>
#include <Common/lofar_map.h>
#include <Common/lofar_string.h>
#include <APL/CAL_Protocol/ACC.h>
#include <APL/CAL_Protocol/SubArray.h>
#include "Source.h"
#include "DipoleModel.h"
#include "SubArrays.h"

#include <GCF/TM/GCF_Control.h>
#include <AMCBase/ConverterClient.h>

namespace LOFAR {
  using GCF::TM::GCFTask;
  using GCF::TM::GCFPort;
  using GCF::TM::GCFTCPPort;
  using GCF::TM::GCFPortInterface;
  namespace CAL {

// forward declarations
class CalibrationAlgorithm;
#ifdef USE_CAL_THREAD
class CalibrationThread;
#endif

class CalServer : public GCFTask
{
public:
	// The constructor of the CalServer task.
	// @param name The name of the task. The name is used for looking
	// up connection establishment information using the GTMNameService and
	// GTMTopologyService classes.
	// @param accs Reference to the global ACC's. These ACC's are shared between
	// the calibration algorithm and the ACMProxy class.
	CalServer(const string& name, ACCs& accs, int argc, char** argv);
	virtual ~CalServer();

	// Adopt the commandline switches
	void parseOptions(int argc, char** argv);

	// The undertaker method deletes dead clients on the m_dead_clients list.
	void undertaker();

	// Remove a client and the associated subarray.
	void remove_client(GCFPortInterface* port);

	// increment RCU usagecounters and enable newly used RCUs
	void _enableRCUs(SubArray*	subarray);

	// decrement RCU usagecounters and disable unused RCUs
	void _disableRCUs(SubArray*	subarray);

	/*@{*/
	// States
	GCFEvent::TResult initial(GCFEvent& e, GCFPortInterface &port);
	GCFEvent::TResult enabled(GCFEvent& e, GCFPortInterface &port);
	/*@}*/

	/*@{*/
	// Handle the CAL_Protocol requests
	GCFEvent::TResult handle_cal_start      (GCFEvent& e, GCFPortInterface &port);
	GCFEvent::TResult handle_cal_stop       (GCFEvent& e, GCFPortInterface &port);
	GCFEvent::TResult handle_cal_subscribe  (GCFEvent& e, GCFPortInterface &port);
	GCFEvent::TResult handle_cal_unsubscribe(GCFEvent& e, GCFPortInterface &port);
	GCFEvent::TResult handle_cal_getsubarray(GCFEvent& e, GCFPortInterface &port);
	/*@}*/

	// Write ACC to file if configured to do so.
	void write_acc();

private:
	AntennaArrays               m_arrays;       // antenna arrays (read from file)
	Sources                     m_sources;      // source catalog (read from file)
	DipoleModels                m_dipolemodels; // dipole model   (read from file)

	SubArrays                   m_subarrays;    // the subarrays (created by clients)
	ACCs&                       m_accs;         // front and back ACC buffers (received from ACMServer)

	CalibrationAlgorithm*       m_cal;          // pointer to the calibration algorithm to use

	AMC::ConverterClient*       m_converter;    // interface for coordinate conversion (Astronomical Measures Conversion)

	// Current sampling frequency of the system.
	double m_sampling_frequency;

	// remember number of RSP boards and number of rcus
	int m_n_rspboards;
	int m_n_rcus;

	// Which instance of the services we should use.
	int32 m_instancenr;

	vector<int>		itsRCUcounts;		// in how many observations an RCU participates

	// Ports
	GCFTCPPort m_acceptor;  // connect point for clients
	GCFPort    m_rspdriver; // connect to RSPDriver for RSP_CONFIG and RSP_SETRCU events

	// Client/Server management member variables.
	map<string, GCFPortInterface*> 	m_clients;      // list of subarraynames with related clients
	list<GCFPortInterface*>			m_dead_clients; // list of disconnected clients

#ifdef USE_CAL_THREAD
	// CalibrationThread
	CalibrationThread* m_calthread;
	pthread_mutex_t    m_globallock;
#endif
};

  }; // namespace CAL
}; // namespace LOFAR

#endif /* CALSERVER_H_ */
