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

#include "CalConstants.h"
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
    m_accs(accs), m_cal(0)
{
  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);

  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, CAL_PROTOCOL);
}

CalServer::~CalServer()
{
  if (m_cal) delete m_cal;
}

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

	  //
	  // Setup calibration algorithm
	  //
	  m_cal = new RemoteStationCalibration(m_sources, m_dipolemodels);

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

	const Timestamp t = Timestamp(timer->sec, timer->usec);
	LOG_INFO_STR("updateAll @ " << t);

	//
	// Swap buffers when all calibrations have finished on the front buffer
	// and the back buffer is not locked and is valid (has been filled by ACMProxy).
	// 
	if (   !m_accs.getFront().isLocked()
	    && !m_accs.getBack().isLocked()
	    && m_accs.getBack().isValid())
	{
	  LOG_INFO("swapping buffers");

	  // start new calibration
	  m_accs.swap();
	  m_accs.getBack().invalidate(); // invalidate
	}

	m_subarrays.calibrate(m_cal, m_accs.getFront());

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

    LOG_DEBUG_STR("m_accs.getBack().getACC().shape()=" << m_accs.getBack().getACC().shape());
    LOG_DEBUG_STR("positions.shape()" << positions.shape());

    ASSERT(m_accs.getBack().getACC().extent(firstDim)   == GET_CONFIG("CalServer.NSUBBANDS", i));
    ASSERT(m_accs.getFront().getACC().extent(secondDim) == positions.extent(secondDim));
    ASSERT(m_accs.getFront().getACC().extent(thirdDim)  == positions.extent(secondDim));
    ASSERT(m_accs.getFront().getACC().extent(fourthDim) == positions.extent(firstDim));
    ASSERT(m_accs.getFront().getACC().extent(fifthDim)  == positions.extent(firstDim));

    // create subarray to calibrate
    SubArray* subarray = new SubArray(start.name,
				      positions,
				      select,
				      start.sampling_frequency,
				      start.nyquist_zone,
				      GET_CONFIG("CalServer.NSUBBANDS", i));

    m_subarrays.add(subarray);

    // calibration will start within one second

#if 0
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

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  ACCs* accs; // the ACC buffers
  accs = new ACCs(GET_CONFIG("CalServer.NSUBBANDS", i),
		  GET_CONFIG("CalServer.NANTENNAS", i),
		  NPOL);

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
