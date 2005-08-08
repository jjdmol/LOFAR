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
#include "CalibrationAlgorithm.h"

#ifdef USE_CAL_THREAD
#include "CalibrationThread.h"
#endif

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
#ifdef USE_CAL_THREAD
    , m_calthread(0)
#endif
{
#ifdef USE_CAL_THREAD
  pthread_mutex_init(&m_globallock, 0);
#endif

  registerProtocol(CAL_PROTOCOL, CAL_PROTOCOL_signalnames);
  m_acceptor.init(*this, "acceptor", GCFPortInterface::MSPP, CAL_PROTOCOL);
}

CalServer::~CalServer()
{
  if (m_cal)       delete m_cal;
#ifdef USE_CAL_THREAD
  if (m_calthread) delete m_calthread;
#endif
}

void CalServer::undertaker()
{
  for (list<GCFPortInterface*>::iterator it = m_dead_clients.begin();
       it != m_dead_clients.end(); ++it)
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
    m_subarrays.schedule_remove(subarray);
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

#ifdef USE_CAL_THREAD
	  pthread_mutex_lock(&m_globallock); // lock for dipolemodels, and sources
#endif

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

#ifdef USE_CAL_THREAD
	  //
	  // Setup calibration thread
	  m_calthread = new CalibrationThread(&m_subarrays, m_cal, m_globallock);

	  pthread_mutex_unlock(&m_globallock); // unlock global lock
#endif

	} catch (Exception e)  {

#ifdef USE_CAL_THREAD
	  pthread_mutex_unlock(&m_globallock); // unlock global lock
#endif
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
	LOG_DEBUG_STR("updateAll @ " << t);

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

#ifdef USE_CAL_THREAD
	  // join previous calibration thread
	  (void)m_calthread->join();
#endif

	  m_subarrays.mutex_lock();
	  undertaker(); // destroy dead clients, done here to prevent possible use of closed port
	  m_subarrays.undertaker();  // remove subarrays scheduled for deletion
	  m_subarrays.creator();     // bring new subarrays to life
	  m_subarrays.mutex_unlock();

	  if (GET_CONFIG("CalServer.WriteACCToFile", i)) write_acc();

#ifdef USE_CAL_THREAD
	  // start calibration thread
	  m_calthread->setACC(&m_accs.getFront());
	  m_calthread->run();
#else
	  m_subarrays.calibrate(m_cal, m_accs.getFront());
	  m_subarrays.updateAll();
#endif
	}

#ifdef USE_CAL_THREAD
	m_subarrays.updateAll();
#endif
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

    for (int i = 0;
	 i < positions.extent(firstDim)*positions.extent(secondDim);
	 i++)
    {
      if (start.subset[i]) select(i/2,i%2) = true;
    }

    LOG_DEBUG_STR("m_accs.getBack().getACC().shape()=" << m_accs.getBack().getACC().shape());
    LOG_DEBUG_STR("positions.shape()" << positions.shape());

    ASSERT(m_accs.getBack().getACC().extent(firstDim)   == GET_CONFIG("CalServer.N_SUBBANDS", i));
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
				      GET_CONFIG("CalServer.N_SUBBANDS", i));

    m_subarrays.schedule_add(subarray);

    // calibration will start within one second
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
  if (m_subarrays.schedule_remove(stop.name)) {
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

void CalServer::write_acc()
{
  time_t now = time(0);
  struct tm* t = localtime(&now);
  char filename[PATH_MAX];
  const Array<std::complex<double>, 5>& acc = m_accs.getFront().getACC();
  Array<std::complex<double>, 3> newacc;

  newacc.resize(acc.extent(firstDim),
		acc.extent(secondDim)*acc.extent(fourthDim),
		acc.extent(thirdDim)*acc.extent(fifthDim));

  for (int s = 0; s < newacc.extent(firstDim); s++)
    for (int i = 0; i < newacc.extent(secondDim); i++)
      for (int j = 0; j < newacc.extent(thirdDim); j++)
	newacc(s,i,j) = acc(s,i%2,j%2,i/2,j/2);

  snprintf(filename, PATH_MAX, "%04d%02d%02d_%02d%02d%02d_acc_%dx%dx%d.dat",
	   t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
	   t->tm_hour, t->tm_min, t->tm_sec,
	   newacc.extent(firstDim),
	   newacc.extent(secondDim),
	   newacc.extent(thirdDim));
  FILE* accfile = fopen(filename, "w");

  if (!accfile) {
    LOG_FATAL_STR("failed to open file: " << filename);
    exit(EXIT_FAILURE);
  }

  if ((size_t)newacc.size() != fwrite(newacc.data(), sizeof(complex<double>), newacc.size(), accfile)) {
    LOG_FATAL_STR("failed to write to file: " << filename);
    exit(EXIT_FAILURE);
  }

  (void)fclose(accfile);
}

int main(int argc, char** argv)
{
  GCFTask::init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  ACCs* accs; // the ACC buffers
  accs = new ACCs(GET_CONFIG("CalServer.N_SUBBANDS", i),
		  GET_CONFIG("CalServer.N_ANTENNAS", i),
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
