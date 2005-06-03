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

// RSP protocol is temporary placeholder for ACM protocol
#define ACM_PROTOCOL            RSP_PROTOCOL
#define ACM_PROTOCOL_signalname RSP_PROTOCOL_signalnames

ACMProxy::ACMProxy(string name, ACCs& accs)
  : GCFTask((State)&ACMProxy::initial, name),
    m_accs(accs)
{
  registerProtocol(RSP_PROTOCOL, RSP_PROTOCOL_signalnames);

  m_acmserver.init(*this, "acmserver", GCFPortInterface::SAP, ACM_PROTOCOL);
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
	if (!m_acmserver.isConnected()) m_acmserver.open();
      }
      break;

    case F_INIT:
      if (!m_accs.getBack().writeLock()) {
	LOG_FATAL("Failed to get initial write lock on ACC back buffer.");
	exit(EXIT_FAILURE);
      }
      break;

    case F_CONNECTED:
      {
	if (m_acmserver.isConnected())
	{
	  port.setTimer(0.0, 1.0); // check every second
	  TRAN(ACMProxy::idle);
	}
      }
      break;

    case F_DISCONNECTED:
      {
	LOG_DEBUG(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
	port.close();
	port.setTimer(2.0);
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

GCFEvent::TResult ACMProxy::idle(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      break;

    case F_CONNECTED:
      {
	LOG_INFO(formatString("CONNECTED: port '%s'", port.getName().c_str()));
      }
      break;

    case F_TIMER:
      {
	GCFTimerEvent* timer = static_cast<GCFTimerEvent*>(&e);

	LOG_INFO_STR("timer @ " << timer->sec);

	if (m_accs.getFront().writeLock())
	{
#if 0
	  ACMGetEvent get;
	  get.timestamp = Timestamp(0,0);
	  get.rcumask.reset();
	  for (int i = 0; i < accs.getFront().getNAntennas() * accs.getFront().getNPol(); i++) {
	    get.rcumask.set(i);
	  }
	  get.subbands.reset();
	  // TODO select all subbands

	  port.send(get);
#endif
	}
      }
      break;

#if 0
    case ACM_GET_ACK:
      TRAN(ACMProxy::receiving);
      break;

#endif

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	// TODO: cleanup

	TRAN(ACMProxy::initial);
      }
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

GCFEvent::TResult ACMProxy::receiving(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch (e.signal)
    {
    case F_ENTRY:
      break;

#if 0
    case ACM_ACM:
      break;

    case ACM_DONE:
      TRAN(ACMProxy::idle);
      break;
#endif

    case F_DISCONNECTED:
      {
	LOG_INFO(formatString("DISCONNECTED: port %s disconnected", port.getName().c_str()));
	port.close();

	// TODO: cleanup

	TRAN(ACMProxy::initial);
      }
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
    }

  return status;
}

GCFEvent::TResult ACMProxy::handle_acm_acm(GCFEvent& /*e*/, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  //accs.getFront().updateACM(acm.subband, acm.timestamp, acm.acm);

  return status;
}

GCFEvent::TResult ACMProxy::handle_acm_done(GCFEvent& /*e*/, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  m_accs.getFront().writeUnlock();

  return status;
}
