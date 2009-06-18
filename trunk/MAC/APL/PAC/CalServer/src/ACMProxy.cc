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

#include <lofar_config.h>
#include <Common/LofarLogger.h>

#include <MACIO/MACServiceInfo.h>

#include <APL/RSP_Protocol/RSP_Protocol.ph>

#include "ACMProxy.h"

// from RTCCommon
#include <APL/RTCCommon/Timestamp.h>
#include <APL/RTCCommon/PSAccess.h>

#include <Common/ParameterSet.h>

using namespace std;
using namespace blitz;
using namespace LOFAR;
using namespace RSP_Protocol;
using namespace CAL;
using namespace RTC;
using namespace GCF::TM;

#define START_DELAY 4

ACMProxy::ACMProxy(string name, ACCs& accs)
  : GCFTask((State)&ACMProxy::initial, name),
    m_accs(accs),
    m_handle(0),
    m_request_subband(0),
    m_update_subband(0),
    m_nrcus(0)
{
	registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_STRINGS);

	m_rspdriver.init(*this, MAC_SVCMASK_RSPDRIVER, GCFPortInterface::SAP, RSP_PROTOCOL);
}

ACMProxy::~ACMProxy()
{}

GCFEvent::TResult ACMProxy::initial(GCFEvent& e, GCFPortInterface& port)
{
	LOG_DEBUG_STR("ACMProxy::initial: " << eventName(e) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch(e.signal) {
	case F_ENTRY: {
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

	case F_CONNECTED: {
		if (m_rspdriver.isConnected()) {
			RSPGetconfigEvent getconfig;
			m_rspdriver.send(getconfig);
		}
	}
	break;

	case RSP_GETCONFIGACK: {
		RSPGetconfigackEvent ack(e);
		m_nrcus = ack.n_rcus;
		m_nrspboards = ack.n_rspboards;
		if (m_nrcus != m_accs.getBack().getNAntennas() * m_accs.getBack().getNPol()) {
			LOG_FATAL("CalServer.N_ANTENNAS does not match value from hardware");
			exit(EXIT_FAILURE);
		}
		TRAN(ACMProxy::idle);
	}
	break;

	case F_DISCONNECTED: {
		// if we get disconnected, but we're in test mode, simply continue to the idle state
		if (GET_CONFIG("CalServer.ACCTestEnable", i)) {
			TRAN(ACMProxy::idle);
		}
		else {  
			LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
			port.close();
			port.setTimer(2.0);
		}
	}
	break;

	case F_TIMER: {
		if (!port.isConnected()) {
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
	LOG_DEBUG_STR("ACMProxy::idle: " << eventName(e) << "@" << port.getName());
	GCFEvent::TResult status = GCFEvent::HANDLED;

	switch (e.signal) {
	case F_ENTRY: {
		m_rspdriver.setTimer(2.0, 2.0); // check every two second
	}
	break;

	case F_CONNECTED: {
		LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
	}
	break;

	case F_TIMER: {
		/*GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);*/

		//
		// start collecting the next ACC if needed
		//
		if (m_accs.getBack().writeLock()) {
			LOG_INFO("collecting next batch of ACMs");

			if (GET_CONFIG("CalServer.ACCTestEnable", i)) {
				const char* testfilename = GET_CONFIG_STRING("CalServer.ACCTestFile");
				const char* dot          = strrchr(testfilename, '.');
				if (dot) {
					dot++;
					if (0 == strncmp(dot, "txt", 3)) {
						if (0 != m_accs.getBack().getFromFile(GET_CONFIG_STRING("CalServer.ACCTestFile"))) {
							LOG_FATAL("Failed to get ACC.");
							exit(EXIT_FAILURE);
						}
					} else if (0 == strncmp(dot, "bin", 3)) {
						if (0 != m_accs.getBack().getFromBinaryFile(GET_CONFIG_STRING("CalServer.ACCTestFile"))) {
							LOG_FATAL("Failed to get ACC.");
							exit(EXIT_FAILURE);
						}
					} else dot = 0;
				}

				if (!dot) {
					LOG_FATAL_STR("CalServer.ACCTestFile must end in '.txt' or '.dat': offending value:" <<
					GET_CONFIG_STRING("CalServer.ACCTestFile"));
					exit(EXIT_FAILURE);
				}
				finalize(true); // done reading from file
			} else {
				if (!GET_CONFIG("CalServer.DisableACMProxy", i)) {
					TRAN(ACMProxy::initializing);
				}
			}
		} else {
			LOG_WARN("failed to get writeLock on ACC backbuffer");
		}
	}
	break;

	case F_DISCONNECTED: {
		LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
		port.close();

		finalize(false);

		TRAN(ACMProxy::initial);
	}
	break;

	case F_EXIT: {
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
 * In this state a few SETSUBBANDS commands are sent to the
 * RSPDriver to select the right subband for cross correlation.
 */
GCFEvent::TResult ACMProxy::initializing(GCFEvent& e, GCFPortInterface& port)
{
	LOG_DEBUG_STR("ACMProxy::initializing: " << eventName(e) << "@" << port.getName());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_ENTRY: {
		RSPSetsubbandsEvent ss;

		m_starttime.setNow();
		m_starttime = m_starttime + (long)START_DELAY; // start START_DELAY seconds from now

		LOG_INFO_STR("starttime for ACM collection: " << m_starttime);

		ss.timestamp = m_starttime;
		ss.rcumask.reset();
		for (int i = 0; i < m_nrcus; i++) {
		  ss.rcumask.set(i);
		}

		m_request_subband = 0;
		m_update_subband = 0;

		ss.subbands.setType(SubbandSelection::XLET);
		ss.subbands().resize(1, 1);
		ss.subbands() = m_request_subband;

		LOG_DEBUG_STR("REQ: XC subband " << m_request_subband << " @ " << ss.timestamp);
		m_rspdriver.send(ss);

		m_request_subband++;
      }
      break;

    case RSP_SETSUBBANDSACK: {
		RSPSetsubbandsackEvent ack(e);

		if (ack.status == RSP_SUCCESS) {
		  if (m_request_subband < START_DELAY) {
			// request next subband
			RSPSetsubbandsEvent ss;
	  
			ss.timestamp = m_starttime + (long)m_request_subband;
			ss.rcumask.reset();
			for (int i = 0; i < m_nrcus; i++) {
			  ss.rcumask.set(i);
			}
	    
			ss.subbands.setType(SubbandSelection::XLET);
			ss.subbands().resize(1, 1);
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
	LOG_DEBUG_STR("ACMProxy::receiving: " << eventName(e) << "@" << port.getName());
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_ENTRY: {
		// subscribe to statistics
		RSPSubxcstatsEvent subxc;

		subxc.timestamp = m_starttime + (long)1; // wait 1 second to get result
		subxc.period = 1;

		m_rspdriver.send(subxc);
      }
      break;

    case RSP_SUBXCSTATSACK: {
		RSPSubxcstatsackEvent ack(e);
		if (ack.status != RSP_SUCCESS) {
		  LOG_FATAL("SUBCXSTATSACK returned error status");
		  exit(EXIT_FAILURE);
		}

		m_handle = ack.handle;
      }
      break;

    case RSP_UPDXCSTATS: {
		RSPUpdxcstatsEvent upd(e);

		if (m_update_subband < GET_CONFIG("CalServer.N_SUBBANDS", i)) {
		  if (m_handle == upd.handle) {
			if (upd.status == RSP_SUCCESS) {

			  LOG_DEBUG_STR("ACK: XC subband " << m_update_subband << " @ " << upd.timestamp);
			  LOG_DEBUG_STR("upd.stats().shape=" << upd.stats().shape());

			  if (upd.timestamp != m_starttime + (long)m_update_subband + (long)1) {
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

		if (m_request_subband < GET_CONFIG("CalServer.N_SUBBANDS", i)) {
		  // request next subband
		  RSPSetsubbandsEvent ss;
		  
		  ss.timestamp = m_starttime + (long)m_request_subband;
		  ss.rcumask.reset();
		  for (int i = 0; i < m_nrcus; i++) {
			ss.rcumask.set(i);
		  }
			
		  ss.subbands.setType(SubbandSelection::XLET);
		  ss.subbands().resize(1, 1);
		  ss.subbands() = m_request_subband;

		  LOG_DEBUG_STR("REQ: XC subband " << m_request_subband << " @ " << ss.timestamp);
		  port.send(ss);

		  m_request_subband++;
		}
      }
      break;

    case RSP_SETSUBBANDSACK: {
		RSPSetsubbandsackEvent ack(e);
		if (ack.status != RSP_SUCCESS) {
		  LOG_FATAL("SETSUBBANDSACK returned error status");
		  exit(EXIT_FAILURE);
		}
      }
      break;

    case F_DISCONNECTED: {
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
	LOG_DEBUG_STR("ACMProxy::unsubscribing: " << eventName(e) << "@" << port.getName());

  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal) {
    case F_ENTRY: {
		  RSPUnsubxcstatsEvent unsub;
		  unsub.handle = m_handle;
		  m_rspdriver.send(unsub);
      }
      break;

    case RSP_UNSUBXCSTATSACK: {
		RSPUnsubxcstatsackEvent ack(e);
		if (ack.status != RSP_SUCCESS) {
		  LOG_FATAL("UNSUBXCSTATSACK returned error status");
		  exit(EXIT_FAILURE);
		}

		// Finished collecting new ACC
		finalize(true);

		LOG_INFO("finished collecting ACMs");
		TRAN(ACMProxy::idle);
      }
      break;

    case F_DISCONNECTED: {
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
	LOG_DEBUG("finalize");
  if (m_accs.getBack().isWriteLocked()) {
    if (success) {
		m_accs.getBack().validate(); // make valid
	}
    m_accs.getBack().writeUnlock();
  } else {
    LOG_WARN("no writelock! this should not happen ");
  }
}
