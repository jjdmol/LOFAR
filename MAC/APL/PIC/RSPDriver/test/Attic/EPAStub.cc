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

#define ETHERTYPE_EPA 0x10FA

EPAStub::EPAStub(string name)
  : GCFTask((State)&EPAStub::initial, name), Test(name)
{
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_signalnames);

  char addrstr[64];
  snprintf(addrstr, 64, "RSPDriver.MAC_ADDR_LCU");

  LOG_INFO("EPAStub constructor");
  
  m_client.init(*this, "client", GCFPortInterface::SAP, EPA_PROTOCOL, true /*raw*/);
  m_client.setAddr(GET_CONFIG_STRING("RSPDriver.IF_NAME"),
		   GET_CONFIG_STRING(addrstr));
  m_client.setEtherType(ETHERTYPE_EPA);

  // set all pointers to 0
  memset(&m_reg, 0, sizeof(m_reg));

  m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].addr    = new char[MEPHeader::RSR_STATUS_SIZE];
  m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].size    = MEPHeader::RSR_STATUS_SIZE;
  m_reg[MEPHeader::RSR][MEPHeader::RSR_VERSION].addr   = new char[MEPHeader::RSR_VERSION_SIZE];
  m_reg[MEPHeader::RSR][MEPHeader::RSR_VERSION].size   = MEPHeader::RSR_VERSION_SIZE;
  m_reg[MEPHeader::TST][MEPHeader::TST_SELFTEST].addr  = new char[MEPHeader::TST_SELFTEST_SIZE];
  m_reg[MEPHeader::TST][MEPHeader::TST_SELFTEST].size  = MEPHeader::TST_SELFTEST_SIZE;
  m_reg[MEPHeader::WG] [MEPHeader::WG_XSETTINGS].addr  = new char[MEPHeader::WG_XSETTINGS_SIZE  * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::WG] [MEPHeader::WG_XSETTINGS].size  = MEPHeader::WG_XSETTINGS_SIZE;
  m_reg[MEPHeader::WG] [MEPHeader::WG_YSETTINGS].addr  = new char[MEPHeader::WG_YSETTINGS_SIZE  * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::WG] [MEPHeader::WG_YSETTINGS].size  = MEPHeader::WG_YSETTINGS_SIZE;
  m_reg[MEPHeader::WG] [MEPHeader::WG_XWAVE].addr      = new char[MEPHeader::WG_XWAVE_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::WG] [MEPHeader::WG_XWAVE].size      = MEPHeader::WG_XWAVE_SIZE;
  m_reg[MEPHeader::WG] [MEPHeader::WG_YWAVE].addr      = new char[MEPHeader::WG_YWAVE_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::WG] [MEPHeader::WG_YWAVE].size      = MEPHeader::WG_YWAVE_SIZE;
  m_reg[MEPHeader::SS] [MEPHeader::SS_SELECT].addr     = new char[MEPHeader::SS_SELECT_SIZE     * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::SS] [MEPHeader::SS_SELECT].size     = MEPHeader::SS_SELECT_SIZE;
  m_reg[MEPHeader::BF] [MEPHeader::BF_XROUT].addr      = new char[MEPHeader::BF_XROUT_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::BF] [MEPHeader::BF_XROUT].size      = MEPHeader::BF_XROUT_SIZE;
  m_reg[MEPHeader::BF] [MEPHeader::BF_XIOUT].addr      = new char[MEPHeader::BF_XIOUT_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::BF] [MEPHeader::BF_XIOUT].size      = MEPHeader::BF_XIOUT_SIZE;
  m_reg[MEPHeader::BF] [MEPHeader::BF_YROUT].addr      = new char[MEPHeader::BF_YROUT_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::BF] [MEPHeader::BF_YROUT].size      = MEPHeader::BF_YROUT_SIZE;
  m_reg[MEPHeader::BF] [MEPHeader::BF_YIOUT].addr      = new char[MEPHeader::BF_YIOUT_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::BF] [MEPHeader::BF_YIOUT].size      = MEPHeader::BF_YIOUT_SIZE;
  m_reg[MEPHeader::BST][MEPHeader::BST_MEAN].addr      = new char[MEPHeader::BST_MEAN_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_MEAN].size      = MEPHeader::BST_MEAN_SIZE;
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER].addr     = new char[MEPHeader::BST_POWER_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER].size     = MEPHeader::BST_POWER_SIZE;
  m_reg[MEPHeader::SST][MEPHeader::SST_MEAN].addr      = new char[MEPHeader::SST_MEAN_SIZE      * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::SST][MEPHeader::SST_MEAN].size      = MEPHeader::SST_MEAN_SIZE;
  m_reg[MEPHeader::SST][MEPHeader::SST_POWER].addr     = new char[MEPHeader::SST_POWER_SIZE     * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::SST][MEPHeader::SST_POWER].size     = MEPHeader::SST_POWER_SIZE;
  m_reg[MEPHeader::RCU][MEPHeader::RCU_SETTINGS].addr  = new char[MEPHeader::RCU_SETTINGS_SIZE  * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_SETTINGS].size  = MEPHeader::RCU_SETTINGS_SIZE;
  m_reg[MEPHeader::CRR][MEPHeader::CRR_SOFTRESET].addr = new char[MEPHeader::CRR_SOFTRESET_SIZE];
  m_reg[MEPHeader::CRR][MEPHeader::CRR_SOFTRESET].size = MEPHeader::CRR_SOFTRESET_SIZE;
  m_reg[MEPHeader::CRR][MEPHeader::CRR_SOFTPPS].addr   = new char[MEPHeader::CRR_SOFTPPS_SIZE];
  m_reg[MEPHeader::CRR][MEPHeader::CRR_SOFTPPS].size   = MEPHeader::CRR_SOFTPPS_SIZE;
  m_reg[MEPHeader::CRB][MEPHeader::CRB_SOFTRESET].addr = new char[MEPHeader::CRB_SOFTRESET_SIZE * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::CRB][MEPHeader::CRB_SOFTRESET].size = MEPHeader::CRB_SOFTRESET_SIZE;
  m_reg[MEPHeader::CRB][MEPHeader::CRB_SOFTPPS].addr   = new char[MEPHeader::CRB_SOFTPPS_SIZE   * GET_CONFIG("RS.N_BLPS", i)];
  m_reg[MEPHeader::CRB][MEPHeader::CRB_SOFTPPS].size   = MEPHeader::CRB_SOFTPPS_SIZE;
  m_reg[MEPHeader::CDO][MEPHeader::CDO_SETTINGS].addr  = new char[MEPHeader::CDO_SETTINGS_SIZE];
  m_reg[MEPHeader::CDO][MEPHeader::CDO_SETTINGS].size  = MEPHeader::CDO_SETTINGS_SIZE;
}

EPAStub::~EPAStub()
{
  for (int pid = 0; pid <= MEPHeader::MAX_PID; pid++)
    for (int reg = 0; reg <= MEPHeader::MAX_REGID; reg++)
    {
      if (m_reg[pid][reg].addr) delete [] m_reg[pid][reg].addr;
    }
}

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
	      status = read_rsr_status(read, port);
	      break;
	    case MEPHeader::RSR_VERSION:
	      LOG_INFO("READ RSR_VERSION");
	      status = read_rsr_version(read, port);
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
	case MEPHeader::SST:
	  {
	    LOG_INFO("READ STATS");
	    status = read_stats(read, port);
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

    //
    // All register write requests arrive as specific signals
    // and are all handled in the same way,
    // copy to register memory and acknowledge with success status.
    //
    case EPA_RSR_STATUS:
    case EPA_RSR_VERSION:
    case EPA_TST_SELFTEST:
    case EPA_CFG_RESET:
    case EPA_CFG_REPROGRAM:
    case EPA_WG_SETTINGS:
    case EPA_WG_WAVE:
    case EPA_SS_SELECT:
    case EPA_BF_COEFS:
    case EPA_STATS:
    case EPA_RCU_SETTINGS:
    case EPA_CRR_SOFTRESET:
    case EPA_CRR_SOFTPPS:
    case EPA_CRB_SOFTRESET:
    case EPA_CRB_SOFTPPS:
    case EPA_CDO_SETTINGS:
    {
      EPAWriteEvent write(event);
      LOG_INFO(formatString("Received event (pid=0x%02x, regid=0x%02x)",
			    write.hdr.m_fields.addr.pid,
			    write.hdr.m_fields.addr.regid));

      uint8 pid   = write.hdr.m_fields.addr.pid;
      uint8 regid = write.hdr.m_fields.addr.regid;

      ASSERT(pid <= MEPHeader::MAX_PID && regid <= MEPHeader::MAX_REGID);
      ASSERT(m_reg[pid][regid].addr);
	
      if (MEPHeader::DST_RSP == write.hdr.m_fields.addr.dstid)
      {
	// copy to RSP register memory
	ASSERT(write.hdr.m_fields.size <= m_reg[pid][regid].size);
	
	memcpy(m_reg[pid][regid].addr + write.hdr.m_fields.offset,
	       &write.payload, write.hdr.m_fields.size);
      }
      else
      {
	// copy to BLP register memory
	uint32 offset = write.hdr.m_fields.addr.dstid * m_reg[pid][regid].size;
	ASSERT(offset + write.hdr.m_fields.size <= m_reg[pid][regid].size * GET_CONFIG("RS.N_BLPS", i));
	memcpy(m_reg[pid][regid].addr + offset,
	       &write.payload, write.hdr.m_fields.size);
      }

      EPAWriteackEvent writeack;
      
      writeack.hdr = write.hdr;
      writeack.hdr.m_fields.type  = MEPHeader::WRITEACK;
      writeack.hdr.m_fields.error = 0;

      port.send(writeack);
    }
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

GCFEvent::TResult EPAStub::read_rsr_status(EPAReadEvent& event, GCFPortInterface& port)
{
  EPARsrStatusEvent rsr_status;

  rsr_status.hdr = event.hdr;
  rsr_status.hdr.m_fields.type  = MEPHeader::READACK;
  rsr_status.hdr.m_fields.error = 0;

  memset(&rsr_status.board, 0, sizeof(EPA_Protocol::BoardStatus));

  port.send(rsr_status);

  return GCFEvent::HANDLED;
}

GCFEvent::TResult EPAStub::read_rsr_version(EPAReadEvent& event, GCFPortInterface& port)
{
  EPARsrVersionEvent rsr_version;

  rsr_version.hdr = event.hdr;
  rsr_version.hdr.m_fields.type  = MEPHeader::READACK;
  rsr_version.hdr.m_fields.error = 0;

  rsr_version.rsp_version = 0x12;
  rsr_version.bp_version  = 0x34;
  rsr_version.ap_version  = 0x56;

  port.send(rsr_version);

  return GCFEvent::HANDLED;
}

GCFEvent::TResult EPAStub::read_stats(EPAReadEvent& event, GCFPortInterface& port)
{
  EPAStatsEvent stats;

  stats.hdr = event.hdr;
  stats.hdr.m_fields.type  = MEPHeader::READACK;
  stats.hdr.m_fields.error = 0;

  for (uint32 i = 0; i < stats.hdr.m_fields.size / sizeof(uint32); i++)
  {
    stats.stat[i] = i + (stats.hdr.m_fields.offset / sizeof(uint32));
  }

  port.send(stats);

  return GCFEvent::HANDLED;
}

void EPAStub::run()
{
  start(); // make initial transition
  GCFTask::run();
}

int main(int argc, char** argv)
{
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

#if 0
  for (int arg = 0; arg < argc; arg++)
  {
    if (!strcmp(argv[arg], "-boardnr"))
    {
      if (arg++ < argc) boardnr = atoi(argv[arg]);
    }
  }
#endif

  LOG_INFO(formatString("Program %s has started", argv[0]));

  Suite s("EPA Firmware Stub", &cerr);
  s.addTest(new EPAStub("EPAStub"));
  s.run();
  long nFail = s.report();
  s.free();

  LOG_INFO("Normal termination of program");

  return nFail;
}
