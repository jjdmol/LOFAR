//#
//#  EPAStub.cc: implementation of EPAStub class
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

//#define EARLY_REPLY

#include "EPA_Protocol.ph"
#include "RawEvent.h"

#include "EPAStub.h"
#include "RSPTestSuite.h"

#include "BeamletWeights.h"

#include <PSAccess.h>
#include <GCF/ParameterSet.h>

#include <iostream>
#include <sys/time.h>
#include <string.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace RSP_Test;
using namespace std;
using namespace LOFAR;

EPAStub::EPAStub(string name, int boardnr)
    : GCFTask((State)&EPAStub::initial, name), Test(name)
{
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  char addrstr[64];
  snprintf(addrstr, 64, "RSPDriver.MAC_ADDR_LCU");
  
  m_client.init(*this, "client", GCFPortInterface::SAP, EPA_PROTOCOL, true /*raw*/);
  m_client.setAddr(GET_CONFIG_STRING("RSPDriver.IF_NAME"),
		   GET_CONFIG_STRING(addrstr));
}

EPAStub::~EPAStub()
{}

GCFEvent::TResult EPAStub::initial(GCFEvent& e, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(e.signal)
  {
    case F_INIT:
    {
    }
    break;

    case F_ENTRY:
    {
      m_client.open();
    }
    break;

    case F_CONNECTED:
    {
      TRAN(EPAStub::connected);
    }
    break;

    case F_DISCONNECTED:
    {
      port.setTimer((long)1);
      port.close();
    }
    break;

    case F_TIMER:
    {
      // try again
      m_client.open();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EPAStub::connected(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
  {
    case F_ENTRY:
    {
      START_TEST("connected", "The connected state of the EPAStub");
    }
    break;

    case F_DATAIN:
    {
      status = RawEvent::dispatch(*this, port);
    }
    break;

    // All register read requests arrive as a general EPA_READ signal
    case EPA_READ:
    {
      EPAReadEvent read(event);
      uint8 pid   = read.hdr.m_fields.addr.pid;
      uint8 regid = read.hdr.m_fields.addr.regid;
      
      switch (pid)
      {
	case MEPHeader::RSR:
	  switch (regid)
	  {
	    case MEPHeader::RSR_STATUS:
	      LOG_INFO("READ RSR_STATUS");
	      break;
	    case MEPHeader::RSR_VERSION:
	      LOG_INFO("READ RSR_VERSION");
	      break;
	  }
	  break;

	case MEPHeader::TST:
	  switch (regid)
	  {
	    case MEPHeader::TST_SELFTEST:
	      LOG_INFO("READ TST_SELFTEST");
	      break;
	  }
	  break;

	case MEPHeader::CFG:
	  switch (regid)
	  {
	    case MEPHeader::CFG_RESET:
	      LOG_INFO("READ CFG_RESET");
	      break;
	    case MEPHeader::CFG_REPROGRAM:
	      LOG_INFO("READ CFG_REPROGRAM");
	      break;
	  }
	  break;

	case MEPHeader::WG:
	  switch (regid)
	  {
	    case MEPHeader::WG_XSETTINGS:
	    case MEPHeader::WG_YSETTINGS:
	      LOG_INFO("READ WG_SETTINGS");
	      break;

	    case MEPHeader::WG_XWAVE:
	    case MEPHeader::WG_YWAVE:
	      LOG_INFO("READ WG_WAVE");
	      break;
	  }
	  break;

	case MEPHeader::SS:
	  switch (regid)
	  {
	    case MEPHeader::SS_SELECT:
	      LOG_INFO("READ SS_SELECT");
	      break;
	  }
	  break;

	case MEPHeader::BF:
	  switch (regid)
	  {
	    case MEPHeader::BF_XROUT:
	    case MEPHeader::BF_XIOUT:
	      LOG_INFO("READ BF_XOUT");
	      break;

	    case MEPHeader::BF_YROUT:
	    case MEPHeader::BF_YIOUT:
	      LOG_INFO("READ BF_YOUT");
	      break;
	  }
	  break;

	case MEPHeader::BST:
	  switch (regid)
	  {
	    case MEPHeader::BST_MEAN:
	      LOG_INFO("READ BST_MEAN");
	      break;
	    case MEPHeader::BST_POWER:
	      LOG_INFO("READ BST_POWER");
	      break;
	  }
	  break;

	case MEPHeader::SST:
	  switch (regid)
	  {
	    case MEPHeader::SST_MEAN:
	      LOG_INFO("READ SST_MEAN");
	      break;
	    case MEPHeader::SST_POWER:
	      LOG_INFO("READ SST_POWER");
	      break;
	  }
	  break;

	case MEPHeader::RCU:
	  switch (regid)
	  {
	    case MEPHeader::RCU_SETTINGS:
	      LOG_INFO("READ RCU_SETTINGS");
	      break;
	  }
	  break;

	case MEPHeader::CRR:
	  switch (regid)
	  {
	    case MEPHeader::CRR_SOFTRESET:
	      LOG_INFO("READ CRR_SOFTRESET");
	      break;
	    case MEPHeader::CRR_SOFTPPS:
	      LOG_INFO("READ CRR_SOFTPPS");
	      break;
	  }
	  break;

	case MEPHeader::CRB:
	  switch (regid)
	  {
	    case MEPHeader::CRB_SOFTRESET:
	      LOG_INFO("READ CRB_SOFTRESET");
	      break;
	    case MEPHeader::CRB_SOFTPPS:
	      LOG_INFO("READ CRB_SOFTPPS");
	      break;
	  }
	  break;

	case MEPHeader::CDO:
	  switch (regid)
	  {
	    case MEPHeader::CDO_SETTINGS:
	      LOG_INFO("READ CDO_SETTINGS");
	      break;
	  }
	  break;
      }
    }
    break;

    // All register write requests arrive as specific signals
    case EPA_RSR_STATUS:
      LOG_INFO("WRITE RSR_STATUS");
      break;
      
    case EPA_RSR_VERSION:
      LOG_INFO("WRITE RSR_VERSIOn");
      break;
      
    case EPA_TST_SELFTEST:
      LOG_INFO("WRITE TST_SELFTEST");
      break;
      
    case EPA_CFG_RESET:
      LOG_INFO("WRITE CFG_RESET");
      break;
      
    case EPA_CFG_REPROGRAM:
      LOG_INFO("WRITE CFG_REPROGRAM");
      break;
      
    case EPA_WG_SETTINGS:
      LOG_INFO("WRITE WG_SETTINGS");
      break;
      
    case EPA_WG_WAVE:
      LOG_INFO("WRITE WG_WAVE");
      break;
      
    case EPA_SS_SELECT:
      LOG_INFO("WRITE SS_SELECT");
      break;
      
    case EPA_BF_COEFS:
      LOG_INFO("WRITE BF_COEFS");
      break;
      
    case EPA_STATS:
      LOG_INFO("WRITE STATS");
      break;
      
    case EPA_RCU_SETTINGS:
      LOG_INFO("WRITE RCU_SETTINGS");
      break;

    case EPA_CRR_SOFTRESET:
      LOG_INFO("WRITE CRR_SOFTRESET");
      break;
      
    case EPA_CRR_SOFTPPS:
      LOG_INFO("WRITE CRR_SOFTPPS");
      break;
      
    case EPA_CRB_SOFTRESET:
      LOG_INFO("WRITE CRB_SOFTRESET");
      break;
      
    case EPA_CRB_SOFTPPS:
      LOG_INFO("WRITE CRB_SOFTPPS");
      break;
      
    case EPA_CDO_SETTINGS:
      LOG_INFO("WRITE CDO_SETTINGS");
      break;
      
    case F_DISCONNECTED:
    {
      port.close();

      TRAN(EPAStub::initial);
    }
    break;

    case F_EXIT:
    {
      STOP_TEST();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EPAStub::final(GCFEvent& event, GCFPortInterface& /*port*/)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;

  switch(event.signal)
  {
    case F_ENTRY:
      GCFTask::stop();
      break;

    case F_EXIT:
      break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

GCFEvent::TResult EPAStub::fwversion(GCFEvent& /*event*/, GCFPortInterface& port)
{
#if 0
  EPARsrVersionEvent version;

  // set the correct header info
  version.hdr.set(MEPHeader::RSR_VERSION_HDR,
		  
  MEP_FWVERSION(version.hdr, MEPHeader::READRES);
  version.rsp_version = (1 << 4) & 2; // version 1.2
  version.bp_version = (3 << 4) & 4;  // version 2.4
  
  for (int i = 0; i < EPA_Protocol::N_AP; i++)
  {
    version.ap_version[i] = (5 << 4) & 6; // version 5.6
  }
  
  port.send(version);

  return GCFEvent::HANDLED;
#endif
}

GCFEvent::TResult EPAStub::rspstatus(GCFEvent& /*event*/, GCFPortInterface& port)
{
#if 0
  EPARspstatusEvent rspstatus;

  // set the correct header info
  MEP_RSPSTATUS(rspstatus.hdr, MEPHeader::READRES);
  memset(&rspstatus.board, 0, MEPHeader::RSPSTATUS_SIZE);

  port.send(rspstatus);

  return GCFEvent::HANDLED;
#endif
}

void EPAStub::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
  int boardnr = 0;
  
  GCFTask::init(argc, argv);

  try
  {
    GCF::ParameterSet::instance()->adoptFile("RemoteStation.conf");
    GCF::ParameterSet::instance()->adoptFile("RSPDriver.conf");
  }
  catch (Exception e)
  {
    cerr << "Failed to load configuration files: " << e.text() << endl;
    exit(EXIT_FAILURE);
  }

  for (int arg = 0; arg < argc; arg++)
  {
    if (!strcmp(argv[arg], "-boardnr"))
    {
      if (arg++ < argc) boardnr = atoi(argv[arg]);
    }
  }

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("EPA Firmware Stub", &cerr);
  s.addTest(new EPAStub("EPAStub", boardnr));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
