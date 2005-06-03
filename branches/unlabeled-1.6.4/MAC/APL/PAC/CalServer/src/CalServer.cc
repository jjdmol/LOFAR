//#
//#  CalServer.cc: implementation of CalServer class
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

// this include needs to be first!
#define DECLARE_SIGNAL_NAMES
#include "CAL_Protocol.ph"

#include "CalServer.h"
#include "SpectralWindow.h"
#include "SubArray.h"
#include "SubArraySubscription.h"
#include "RemoteStationCalibration.h"

#include "ACMProxy.h"

// from RTCCommon
#include "Timestamp.h"
#include "PSAccess.h"

#ifndef CAL_SYSCONF
#define CAL_SYSCONF "."
#endif

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/ParameterSet.h>
#include <fstream>
#include <signal.h>

#include <blitz/array.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;

using namespace CAL_Protocol;

CalServer::CalServer(string name, ACCs& accs)
  : GCFTask((State)&CalServer::initial, name),
    m_accs(accs)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, CAL_PROTOCOL);
}

CalServer::~CalServer()
{}

void CalServer::undertaker()
{
  for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
       it != m_dead_clients.end();
       it++)
  {
    delete (*it);
  }
  m_dead_clients.clear();
}

void CalServer::remove_client(GCFPortInterface* port)
{
  ASSERT(0 != port);

  map<GCFPortInterface*, string>::iterator p = m_clients.find(port);
  ASSERT(p != m_clients.end());

  SubArray* subarray = m_subarrays.getByName(m_clients[port]);
  if (subarray) {
    m_subarrays.remove(subarray);
  }

  m_clients.erase(port);
  m_dead_clients.push_back(port);

}

GCFEvent::TResult CalServer::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
    {
    case F_INIT:
      {
	try { 

	  //
	  // load the dipole models
	  //
	  m_dipolemodels.getAll(string(CAL_SYSCONF "/") + string(GET_CONFIG_STRING("CalServer.DipoleModelFile")));

	  //
	  // load the source catalog
	  //
	  m_sources.getAll(string(CAL_SYSCONF "/") + string(GET_CONFIG_STRING("CalServer.SourceCatalogFile")));

	  //
	  // Load antenna arrays
	  //
	  m_arrays.getAll(string(CAL_SYSCONF "/") + string(GET_CONFIG_STRING("CalServer.AntennaArraysFile")));

	} catch (Exception e)  {

	  LOG_ERROR_STR("Failed to load configuration files: " << e);
	  exit(EXIT_FAILURE);

	}
      }
      break;

    case F_ENTRY:
      {
	if (!m_acceptor.isConnected()) m_acceptor.open();
      }
      break;

    case F_CONNECTED:
      {
	if (m_acceptor.isConnected()) TRAN(CalServer::enabled);
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
	port.close();
	port.setTimer(3.0);
      }
      break;

    case F_TIMER:
      {
	if (!port.isConnected())
	  {
	    LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
	    port.open();
	  }
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult CalServer::enabled(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  undertaker(); // destroy dead clients

  switch (e.signal)
    {
    case F_ENTRY:
      {
	m_acceptor.setTimer(0.0, 1.0);
      }
      break;

    case F_ACCEPT_REQ:
      {
 	GCFTCPPort* client = new GCFTCPPort();
 	client->init(*this, "client", GCFPortInterface::SPP, CAL_PROTOCOL);
	m_acceptor.accept(*client);
	m_clients[client] = ""; // empty string to indicate there is a connection, but no subarray yet

	LOG_INFO(formatString("NEW CLIENT CONNECTED: %d clients connected", m_clients.size()));
      }
      break;
      
    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	//const Timestamp t = Timestamp(timer->sec, timer->usec);
	LOG_INFO_STR("updateAll @ " << timer->sec);

	m_subarrays.updateAll();

      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	if (&m_acceptor == &port)
	  {
	    TRAN(CalServer::initial);
	  }
	else
	  {
	    // destroy subarray
	    remove_client(&port);
	  }
      }
      break;

    case CAL_START:
      status = handle_cal_start(e, port);
      break;

    case CAL_STOP:
      status = handle_cal_stop(e, port);
      break;

    case CAL_SUBSCRIBE:
      status = handle_cal_subscribe(e, port);
      break;

    case CAL_UNSUBSCRIBE:
      status = handle_cal_unsubscribe(e, port);
      break;

    case F_EXIT:
      {
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult CalServer::handle_cal_start(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  CALStartEvent start(e);
  CALStartackEvent ack;

  ack.status = SUCCESS; // assume succes, until otherwise
  ack.name = start.name;

  // find parent AntennaArray
  const AntennaArray* parent = m_arrays.getByName(start.parent);

  if ("" != m_clients[&port]) {
    LOG_ERROR_STR("A subarray has already been registered: name=" << m_clients[&port]);
    LOG_ERROR("Only one active subarray per client supported.");
    ack.status = ERR_RANGE;
  } else if ("" == string(start.name)) {
    LOG_ERROR("Empty subarray name.");
    ack.status = ERR_RANGE;
  } else if (!parent) {

    // parent not found, set error status
    LOG_ERROR_STR("Parent array '" << start.parent << "' not found.");
    ack.status = ERR_NO_PARENT;

  } else {

    m_clients[&port] = string(start.name); // register subarray with port

    const Array<double, 3>& positions = parent->getAntennaPos();
    Array<bool, 2> select;
    select.resize(positions.extent(firstDim),
		  positions.extent(secondDim));

    select = true;

    // create subarray to calibrate
    SubArray* subarray = new SubArray(start.name,
				      positions,
				      select,
				      start.sampling_frequency,
				      start.nyquist_zone,
				      CAL_Protocol::N_SUBBANDS);

    m_subarrays.add(subarray);

#if 1
    subarray->calibrate(0, m_accs.getFront());
#else
    //
    // load the ACC
    //
    m_acc = ACCLoader::loadFromFile(CAL_SYSCONF "/ACC.conf");
    
    if (!m_acc) {
      LOG_ERROR("Failed to load ACC matrix.");
      exit(EXIT_FAILURE);
    }

    //
    // Create the calibration algorithm and
    // call the calibrate routine of the subarray
    // with the ACC.
    //
    const DipoleModel* model = m_dipolemodels.getByName("LBAntenna");
    if (!model) {
      LOG_FATAL("Failed to load dipolemodel 'LBAntenna'");
      exit(EXIT_FAILURE);
    }
    RemoteStationCalibration cal(m_sources, *model);
    subarray->calibrate(&cal, *m_acc);
#endif
  }

  port.send(ack); // send ack

  return status;
}

GCFEvent::TResult CalServer::handle_cal_stop(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  CALStopEvent stop(e);
  CALStopackEvent ack;
  ack.name = stop.name;
  ack.status = SUCCESS;
  
  // destroy subarray, what do we do with the observers?
  if (m_subarrays.remove(stop.name)) {
    m_clients[&port] = ""; // unregister subarray name
  } else {
    ack.status = ERR_NO_SUBARRAY; // subarray not found
  }

  port.send(ack);

  return status;
}

GCFEvent::TResult CalServer::handle_cal_subscribe(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  CALSubscribeEvent subscribe(e);
  CALSubscribeackEvent ack;
  ack.status = SUCCESS;

  // get subarray by name
  SubArray* subarray = m_subarrays.getByName(subscribe.name);

  if (subarray) {

    // create subscription
    SubArraySubscription* subscription = new SubArraySubscription(subarray,
								  subscribe.subbandset,
								  port);

    ack.handle = (uint32)subscription;
								
    // attach subscription to the subarray
    subarray->attach(subscription);

  } else {

    ack.status = ERR_NO_SUBARRAY;

  }

  port.send(ack);

  return status;
}

GCFEvent::TResult CalServer::handle_cal_unsubscribe(GCFEvent& e, GCFPortInterface &port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  CALUnsubscribeEvent unsubscribe(e);

  // create ack
  CALUnsubscribeackEvent ack;
  ack.handle = unsubscribe.handle;
  ack.status = SUCCESS;

  // find associated subarray
  SubArray* subarray = m_subarrays.getByName(m_clients[&port]);
  if (subarray) {

    // detach subscription, this destroys the subscription
    subarray->detach((SubArraySubscription*)unsubscribe.handle);

    // handle is no longer valid

  } else {

    ack.status = ERR_NO_SUBARRAY;

  }
  port.send(ack);

  return status;
}

void CalServer::calibrate()
{
#if 0
  //
  // Load configuration files.
  //
  try 
    {
      ParameterSet ps(CAL_SYSCONF "/SpectralWindow.conf");

      //
      // load the spectral window configurations
      //
      m_spws = SPWLoader::loadFromBlitzStrings(ps["SpectralWindow.params"],
					       ps["SpectralWindow.names"]);

      //
      // load the dipole models
      //
      m_dipolemodels.getAll(CAL_SYSCONF "/DipoleModel.conf");

      //
      // load the source catalog
      //
      m_sources.getAll(CAL_SYSCONF "/SourceCatalog.conf");

      //
      // Load antenna arrays
      //
      m_arrays.getAll(CAL_SYSCONF "/AntennaArrays.conf");

      //
      // load the ACC
      //
      m_acc = ACCLoader::loadFromFile(CAL_SYSCONF "/ACC.conf");

      if (!m_acc)
	{
	  LOG_ERROR("Failed to load ACC matrix.");
	  exit(EXIT_FAILURE);
	}

      //cout << "sizeof(ACC)=" << m_acc->getSize() * sizeof(complex<double>) << endl;

#if 0
      ofstream accstream("acc.out");
      if (accstream.is_open())
	{
	  accstream << m_acc->getACC();
	}
#endif
    }
  catch (Exception e) 
    {
      LOG_ERROR_STR("Failed to load configuration files: " << e);
      exit(EXIT_FAILURE);
    }

#if 0
  for (unsigned int i = 0; i < m_spws.size(); i++)
    {
      cout << "SPW[" << i << "]=" << m_spws[i].getName() << endl;
    }
#endif

  //
  // Get the low-band antenna array
  //
  const AntennaArray* array = m_arrays.getByName("LBA");
  if (!array) {
    LOG_FATAL("Failed to load the antenna array definition 'LBA'");
    exit(EXIT_FAILURE);
  }

  //
  // Dimensions of the antenna array
  //
  int nantennas = array->getAntennaPos().extent(firstDim);
  int npol      = array->getAntennaPos().extent(secondDim);

  //
  // Create the FTS-1 subarray, with spectral window 0 (0 - 80 MHz)
  // 
  Array<bool,2> select(nantennas, npol);
  select = true;
  SubArray fts1("FTS-1", array->getAntennaPos(), select, m_spws[0]);

  //
  // Create the calibration algorithm and
  // call the calibrate routine of the subarray
  // with the ACC.
  //
  const DipoleModel* model = m_dipolemodels.getByName("LBAntenna");
  if (!model) {
    LOG_FATAL("Failed to load dipolemodel 'LBAntenna'");
    exit(EXIT_FAILURE);
  }
  RemoteStationCalibration cal(m_sources, *model);
  fts1.calibrate(&cal, *m_acc);

  //
  // Sanity check on dimensions of the various arrays
  //
  // Number of antennas and polarizations must be equal on all related arrays.
  //
  ASSERT(m_acc->getACC().extent(firstDim) == m_spws[0].getNumSubbands());
  ASSERT(m_acc->getACC().extent(secondDim) == nantennas);
  ASSERT(m_acc->getACC().extent(thirdDim) == nantennas);
  ASSERT(m_acc->getACC().extent(fourthDim) == npol);
  ASSERT(m_acc->getACC().extent(fifthDim) == npol);

  //
  // Save the calibration gains and quality matrices
  //
  const AntennaGains* gains = 0;
  if (fts1.getGains(gains, SubArray::BACK))
    {
      // save gains matrix
      ofstream gainsout("gains.out");
      if (gainsout.is_open())
	{
	  gainsout << gains->getGains();
	}
      else
	{
	  LOG_ERROR("Failed to open output file: gains.out");
	  exit(EXIT_FAILURE);
	}

      // save quality matrix
      ofstream qualityout("quality.out");
      if (qualityout.is_open())
	{
	  qualityout << gains->getQuality();
	}
      else
	{
	  LOG_ERROR("Failed to open output file: quality.out");
	  exit(EXIT_FAILURE);
	}
    }
  else
    {
      LOG_ERROR("Calibration has not yet completed.");
      exit(EXIT_FAILURE);
    }
#endif
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  ACCs* accs; // the ACC buffers
  accs = new ACCs(GET_CONFIG("CalServer.nsubbands", i),
		  GET_CONFIG("CalServer.nantennas", i),
		  GET_CONFIG("CalServer.npol", i));

  if (!accs) {
    LOG_FATAL("Failed to allocate memory for the ACC arrays.");
    exit(EXIT_FAILURE);
  }

  //
  // create CalServer and ACMProxy tasks
  // they communicate via the ACCs instance
  //
  CalServer cal     ("CalServer", *accs);
  ACMProxy  acmproxy("ACMProxy",  *accs);

  cal.start();      // make initial transition
  acmproxy.start(); // make initial transition

  try
  {
    GCFTask::run();
  }
  catch (Exception e)
  {
    LOG_ERROR_STR("Exception: " << e.text());
    exit(EXIT_FAILURE);
  }

  delete accs;

  LOG_INFO("Normal termination of program");

  return 0;
}
