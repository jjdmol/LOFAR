//#
//#  ACMProxy.cc: implementation of ACMProxy class
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

#include "RSP_Protocol.ph"

#include "ACMProxy.h"

// from RTCCommon
#include "Timestamp.h"
#include "PSAccess.h"

// #ifndef CAL_SYSCONF
// #define CAL_SYSCONF "."
// #endif

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <GCF/ParameterSet.h>

using namespace RSP_Protocol;

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace CAL;
using namespace RTC;

#define START_DELAY 4

#ifndef CAL_SYSCONF
#define CAL_SYSCONF "."
#endif

ACMProxy::ACMProxy(string name, ACCs& accs)
  : GCFTask((State)&ACMProxy::initial, name),
    m_accs(accs),
    m_handle(0),
    m_request_subband(0),
    m_update_subband(0)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_rspdriver.init(*this, "rspdriver", GCFPortInterface::SAP, RSP_PROTOCOL);
}

ACMProxy::~ACMProxy()
{}

GCFEvent::TResult ACMProxy::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch(e.signal)
    {
    case F_ENTRY:
      {
	m_handle = 0;
	m_starttime = Timestamp(0,0);
	m_request_subband = 0;
	m_update_subband = 0;

	LOG_DEBUG("opening port: m_rspdriver");
	m_rspdriver.open();
	
	//
	// When TRAN(CalServer::initial) is done when handling F_DISCONNECTED
	// in any of the other states, then F_ENTRY of ::initial will be followed
	// by a port.close() in the GCF framework which cancels the open on the previous line.
	// Setting this timer will make sure the port is opened again.
	//
	m_rspdriver.setTimer(0.0);
      }
      break;

    case F_INIT:
      break;

    case F_CONNECTED:
      {
	if (m_rspdriver.isConnected())
	{
	  TRAN(ACMProxy::idle);
	}
      }
      break;

    case F_DISCONNECTED:
      {
	// if we get disconnected, but we're in test mode, simply continue to the idle state
	if (GET_CONFIG("CalServer.ACCTestEnable", i))
	{
	  TRAN(ACMProxy::idle);
	}
	else
	{  
	  LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
	  port.close();
	  port.setTimer(2.0);
	}
      }
      break;

    case F_TIMER:
      {
	if (!port.isConnected())
	  {
	    LOG_DEBUG(formatString("open port '%s'", port.getName().c_str()));
	    port.open();
	  }
      }
      break;

    case F_EXIT:
      // stop timer, we're connected
      m_rspdriver.cancelAllTimers();
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult ACMProxy::idle(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	m_rspdriver.setTimer(2.0, 2.0); // check every two second
      }
      break;

    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	/*GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);*/

	//
	// start collecting the next ACC if needed
	//
	if (m_accs.getBack().writeLock()) {
	  LOG_INFO("collecting next batch of ACMs");

	  if (GET_CONFIG("CalServer.ACCTestEnable", i)) {
	    if (0 != m_accs.getBack().getFromFile(GET_CONFIG_STRING("CalServer.ACCTestFile"))) {
	      LOG_FATAL("Failed to get ACC.");
	      exit(EXIT_FAILURE);
	    }
	    finalize(true); // done reading from file
	  } else {
	    TRAN(ACMProxy::initializing);
	  }
	} else {
	  LOG_WARN("failed to get writeLock on ACC backbuffer");
	}
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	finalize(false);

	TRAN(ACMProxy::initial);
      }
      break;

    case F_EXIT:
      {
	// stop timer, it is not needed in the next state
	m_rspdriver.cancelAllTimers();
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

/**
 * In this state a few SETSUBBANDS command are sent to the
 * RSPDriver to select the right subband for cross correlation.
 */
GCFEvent::TResult ACMProxy::initializing(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	RSPSetsubbandsEvent ss;

	m_starttime.setNow();
	m_starttime = m_starttime + START_DELAY; // start START_DELAY seconds from now

	LOG_INFO_STR("starttime for ACM collection: " << m_starttime);

	ss.timestamp = m_starttime;
	ss.rcumask.reset();
	for (int i = 0; i < 8; i++) {
	  ss.rcumask.set(i);
	}

	m_request_subband = 0;
	m_update_subband = 0;
	ss.subbands().resize(1, 4*2);
	ss.subbands() = m_request_subband;

	LOG_DEBUG_STR("REQ: XC subband " << m_request_subband << " @ " << ss.timestamp);
	m_rspdriver.send(ss);

	m_request_subband++;
      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);

	if (SUCCESS == ack.status) {

	  if (m_request_subband < START_DELAY) {
	    // request next subband
	    RSPSetsubbandsEvent ss;
	  
	    ss.timestamp = m_starttime + m_request_subband;
	    ss.rcumask.reset();
	    for (int i = 0; i < 8; i++) {
	      ss.rcumask.set(i);
	    }
	    
	    ss.subbands().resize(1, 4*2);
	    ss.subbands() = m_request_subband;

	    LOG_DEBUG_STR("REQ: XC subband " << m_request_subband << " @ " << ss.timestamp);
	    port.send(ss);

	    m_request_subband++;

	  } else {
	    TRAN(ACMProxy::receiving);
	  }
	} else {
	  LOG_FATAL("SETSUBBANDSACK returned error status");
	  exit(EXIT_FAILURE);
	}
      }
      break;

    case F_EXIT:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult ACMProxy::receiving(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	// subscribe to statistics
	RSPSubxcstatsEvent subxc;

	subxc.timestamp = m_starttime + 1; // wait 1 second to get result
	subxc.rcumask.reset();

	LOG_DEBUG_STR("nRCU's=" << m_accs.getBack().getNAntennas() * m_accs.getBack().getNPol());
	for (int i = 0; i < m_accs.getBack().getNAntennas() * m_accs.getBack().getNPol(); i++) {
	  subxc.rcumask.set(i);
	}
	subxc.period = 1;

	m_rspdriver.send(subxc);
      }
      break;

    case RSP_SUBXCSTATSACK:
      {
	RSPSubxcstatsackEvent ack(e);

	if (SUCCESS != ack.status) {
	  LOG_FATAL("SUBCXSTATSACK returned error status");
	  exit(EXIT_FAILURE);
	}

	m_handle = ack.handle;
      }
      break;

    case RSP_UPDXCSTATS:
      {
	RSPUpdxcstatsEvent upd(e);

	if (m_update_subband < GET_CONFIG("CalServer.NSUBBANDS", i)) {
	  if (m_handle == upd.handle) {
	    if (SUCCESS == upd.status) {

	      LOG_DEBUG_STR("ACK: XC subband " << m_update_subband << " @ " << upd.timestamp);
	      LOG_DEBUG_STR("upd.stats().shape=" << upd.stats().shape());

	      if (upd.timestamp != m_starttime + m_update_subband + 1) {
		LOG_WARN("incorrect timestamp on XC statistics");
	      }
	      
	      m_accs.getBack().updateACM(m_update_subband, upd.timestamp, upd.stats());
	    } else {
	      LOG_FATAL("UPDXCSTATS returned error code");
	      exit(EXIT_FAILURE);
	    }
	  } else {
	    LOG_WARN("Received UPDXCSTATS event with unknown handle.");
	  }
	  m_update_subband++;
	} else {
	  TRAN(ACMProxy::unsubscribing);
	}

	if (m_request_subband < GET_CONFIG("CalServer.NSUBBANDS", i)) {
	  // request next subband
	  RSPSetsubbandsEvent ss;
	  
	  ss.timestamp = m_starttime + m_request_subband;
	  ss.rcumask.reset();
	  for (int i = 0; i < 8; i++) {
	    ss.rcumask.set(i);
	  }
	    
	  ss.subbands().resize(1, 4*2);
	  ss.subbands() = m_request_subband;

	  LOG_DEBUG_STR("REQ: XC subband " << m_request_subband << " @ " << ss.timestamp);
	  port.send(ss);

	  m_request_subband++;
	}
      }
      break;

    case RSP_SETSUBBANDSACK:
      {
	RSPSetsubbandsackEvent ack(e);
	if (SUCCESS != ack.status) {
	  LOG_FATAL("SETSUBBANDSACK returned error status");
	  exit(EXIT_FAILURE);
	}
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	finalize(false);

	TRAN(ACMProxy::initial);
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult ACMProxy::unsubscribing(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      {
	  RSPUnsubxcstatsEvent unsub;
	  unsub.handle = m_handle;
	  m_rspdriver.send(unsub);
      }
      break;

    case RSP_UNSUBXCSTATSACK:
      {
	RSPUnsubxcstatsackEvent ack(e);

	if (SUCCESS != ack.status) {
	  LOG_FATAL("UNSUBXCSTATSACK returned error status");
	  exit(EXIT_FAILURE);
	}

	// Finished collecting new ACC
	finalize(true);

	LOG_INFO("finished collecting ACMs");
	TRAN(ACMProxy::idle);
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	finalize(false);

	TRAN(ACMProxy::initial);
      }
      break;

    case F_EXIT:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

void ACMProxy::finalize(bool success)
{
  if (m_accs.getBack().isWriteLocked()) {
    if (success) m_accs.getBack().validate(); // make valid
    m_accs.getBack().writeUnlock();
  } else {
    LOG_WARN("no writelock! this should not happen ");
  }
}
