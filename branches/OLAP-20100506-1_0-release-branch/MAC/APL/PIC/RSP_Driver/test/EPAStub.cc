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

#include <lofar_config.h>
#include <Common/LofarLogger.h>
#include <Common/ParameterSet.h>

#include <GCF/TM/GCF_Scheduler.h>

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/BeamletWeights.h>
#include <APL/RTCCommon/PSAccess.h>

#include "RawEvent.h"
#include "EPAStub.h"


#include <iostream>
#include <sys/time.h>
#include <string.h>
#include <math.h>

using namespace std;
namespace LOFAR {
  using namespace GCF::TM;
  namespace RSP_Test {

#define ETHERTYPE_EPA 0x10FA

EPAStub::EPAStub(string name)
  : GCFTask((State)&EPAStub::initial, name), Test(name)
{
  registerProtocol(EPA_PROTOCOL, EPA_PROTOCOL_STRINGS);

  char addrstr[64];
  snprintf(addrstr, 64, "EPAStub.MAC_ADDR_RSPDRIVER");

  LOG_INFO("EPAStub constructor");
  
  m_client.init(*this, "EPAStub:client", GCFPortInterface::SAP, EPA_PROTOCOL, true /*raw*/);
  m_client.setAddr(GET_CONFIG_STRING("EPAStub.IF_NAME"),
		   GET_CONFIG_STRING(addrstr));
  m_client.setEtherType(ETHERTYPE_EPA);

  // set all addr and sizes to 0
  memset(&m_reg, 0, sizeof(m_reg));

  m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].addr  = new char[MEPHeader::RSR_STATUS_SIZE];
  m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].size  = MEPHeader::RSR_STATUS_SIZE;
  m_reg[MEPHeader::RSR][MEPHeader::RSR_VERSION].addr = new char[MEPHeader::RSR_VERSION_SIZE * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RSR][MEPHeader::RSR_VERSION].size = MEPHeader::RSR_VERSION_SIZE;

  m_reg[MEPHeader::RSU][MEPHeader::RSU_FLASHRW].addr     = new char[MEPHeader::RSU_FLASHRW_SIZE];
  m_reg[MEPHeader::RSU][MEPHeader::RSU_FLASHRW].size     = MEPHeader::RSU_FLASHRW_SIZE;
  m_reg[MEPHeader::RSU][MEPHeader::RSU_FLASHERASE].addr  = new char[MEPHeader::RSU_FLASHERASE_SIZE];
  m_reg[MEPHeader::RSU][MEPHeader::RSU_FLASHERASE].size  = MEPHeader::RSU_FLASHERASE_SIZE;
  m_reg[MEPHeader::RSU][MEPHeader::RSU_RECONFIGURE].addr = new char[MEPHeader::RSU_RECONFIGURE_SIZE];
  m_reg[MEPHeader::RSU][MEPHeader::RSU_RECONFIGURE].size = MEPHeader::RSU_RECONFIGURE_SIZE;
  m_reg[MEPHeader::RSU][MEPHeader::RSU_RESET].addr       = new char[MEPHeader::RSU_RESET_SIZE];
  m_reg[MEPHeader::RSU][MEPHeader::RSU_RESET].size       = MEPHeader::RSU_RESET_SIZE;

  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGX].addr      = new char[MEPHeader::DIAG_WGX_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGX].size      = MEPHeader::DIAG_WGX_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGY].addr      = new char[MEPHeader::DIAG_WGY_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGY].size      = MEPHeader::DIAG_WGY_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGXWAVE].addr  = new char[MEPHeader::DIAG_WGXWAVE_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGXWAVE].size  = MEPHeader::DIAG_WGXWAVE_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGYWAVE].addr  = new char[MEPHeader::DIAG_WGYWAVE_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_WGYWAVE].size  = MEPHeader::DIAG_WGYWAVE_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_BYPASS].addr   = new char[MEPHeader::DIAG_BYPASS_SIZE   * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_BYPASS].size   = MEPHeader::DIAG_BYPASS_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_RESULTS].addr  = new char[MEPHeader::DIAG_RESULTS_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_RESULTS].size  = MEPHeader::DIAG_RESULTS_SIZE;
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_SELFTEST].addr = new char[MEPHeader::DIAG_SELFTEST_SIZE * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::DIAG][MEPHeader::DIAG_SELFTEST].size = MEPHeader::DIAG_SELFTEST_SIZE;

  m_reg[MEPHeader::SS][MEPHeader::SS_SELECT].addr     = new char[MEPHeader::SS_SELECT_SIZE     * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::SS][MEPHeader::SS_SELECT].size     = MEPHeader::SS_SELECT_SIZE;

  m_reg[MEPHeader::BF][MEPHeader::BF_XROUT].addr      = new char[MEPHeader::BF_XROUT_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::BF][MEPHeader::BF_XROUT].size      = MEPHeader::BF_XROUT_SIZE;
  m_reg[MEPHeader::BF][MEPHeader::BF_XIOUT].addr      = new char[MEPHeader::BF_XIOUT_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::BF][MEPHeader::BF_XIOUT].size      = MEPHeader::BF_XIOUT_SIZE;
  m_reg[MEPHeader::BF][MEPHeader::BF_YROUT].addr      = new char[MEPHeader::BF_YROUT_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::BF][MEPHeader::BF_YROUT].size      = MEPHeader::BF_YROUT_SIZE;
  m_reg[MEPHeader::BF][MEPHeader::BF_YIOUT].addr      = new char[MEPHeader::BF_YIOUT_SIZE      * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::BF][MEPHeader::BF_YIOUT].size      = MEPHeader::BF_YIOUT_SIZE;

  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_0].addr = new char[MEPHeader::BST_POWER_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_0].size = MEPHeader::BST_POWER_SIZE;
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_1].addr = new char[MEPHeader::BST_POWER_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_1].size = MEPHeader::BST_POWER_SIZE;
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_2].addr = new char[MEPHeader::BST_POWER_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_2].size = MEPHeader::BST_POWER_SIZE;
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_3].addr = new char[MEPHeader::BST_POWER_SIZE];
  m_reg[MEPHeader::BST][MEPHeader::BST_POWER_LANE_3].size = MEPHeader::BST_POWER_SIZE;

  m_reg[MEPHeader::SST][MEPHeader::SST_POWER].addr     = new char[MEPHeader::SST_POWER_SIZE     * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::SST][MEPHeader::SST_POWER].size     = MEPHeader::SST_POWER_SIZE;

  m_reg[MEPHeader::RCU][MEPHeader::RCU_SETTINGS].addr  = new char[MEPHeader::RCU_SETTINGS_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_SETTINGS].size  = MEPHeader::RCU_SETTINGS_SIZE;
  m_reg[MEPHeader::RCU][MEPHeader::RCU_PROTOCOLX].addr = new char[MEPHeader::RCU_PROTOCOL_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_PROTOCOLX].size = MEPHeader::RCU_PROTOCOL_SIZE;
  m_reg[MEPHeader::RCU][MEPHeader::RCU_RESULTX].addr  = new char[MEPHeader::RCU_RESULT_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_RESULTX].size  = MEPHeader::RCU_RESULT_SIZE;
  m_reg[MEPHeader::RCU][MEPHeader::RCU_PROTOCOLY].addr = new char[MEPHeader::RCU_PROTOCOL_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_PROTOCOLY].size = MEPHeader::RCU_PROTOCOL_SIZE;
  m_reg[MEPHeader::RCU][MEPHeader::RCU_RESULTY].addr  = new char[MEPHeader::RCU_RESULT_SIZE  * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::RCU][MEPHeader::RCU_RESULTY].size  = MEPHeader::RCU_RESULT_SIZE;

  // CR_CONTROL register for all AP's and the BP (hence the + 1)
  m_reg[MEPHeader::CR][MEPHeader::CR_SOFTCLEAR].addr   = new char[MEPHeader::CR_CONTROL_SIZE * (GET_CONFIG("EPAStub.N_BLPS", i) + 1)];
  m_reg[MEPHeader::CR][MEPHeader::CR_SOFTCLEAR].size   = MEPHeader::CR_CONTROL_SIZE;
  m_reg[MEPHeader::CR][MEPHeader::CR_SOFTSYNC].addr    = new char[MEPHeader::CR_CONTROL_SIZE * (GET_CONFIG("EPAStub.N_BLPS", i) + 1)];
  m_reg[MEPHeader::CR][MEPHeader::CR_SOFTSYNC].size    = MEPHeader::CR_CONTROL_SIZE;
  m_reg[MEPHeader::CR][MEPHeader::CR_SYNCDISABLE].addr = new char[MEPHeader::CR_CONTROL_SIZE * (GET_CONFIG("EPAStub.N_BLPS", i) + 1)];
  m_reg[MEPHeader::CR][MEPHeader::CR_SYNCDISABLE].size = MEPHeader::CR_CONTROL_SIZE;

  m_reg[MEPHeader::CDO][MEPHeader::CDO_SETTINGS].addr  = new char[MEPHeader::CDO_SETTINGS_SIZE];
  m_reg[MEPHeader::CDO][MEPHeader::CDO_SETTINGS].size  = MEPHeader::CDO_SETTINGS_SIZE;
  m_reg[MEPHeader::CDO][MEPHeader::CDO_HEADER].addr    = new char[MEPHeader::CDO_HEADER_SIZE];
  m_reg[MEPHeader::CDO][MEPHeader::CDO_HEADER].size    = MEPHeader::CDO_HEADER_SIZE;
  
  for (int i = 0; i < MEPHeader::XST_NR_STATS; i++) {
    m_reg[MEPHeader::XST][i].addr = new char[MEPHeader::XST_STATS_SIZE];
    m_reg[MEPHeader::XST][i].size = MEPHeader::XST_STATS_SIZE;
  }

  m_reg[MEPHeader::BS][MEPHeader::BS_NOF_SAMPLES_PER_SYNC].addr  = new char[MEPHeader::BS_NOF_SAMPLES_PER_SYNC_SIZE * GET_CONFIG("EPAStub.N_BLPS", i)];
  m_reg[MEPHeader::BS][MEPHeader::BS_NOF_SAMPLES_PER_SYNC].size  = MEPHeader::BS_NOF_SAMPLES_PER_SYNC_SIZE;

  m_reg[MEPHeader::TDS][MEPHeader::TDS_PROTOCOL].addr = new char[MEPHeader::TDS_PROTOCOL_SIZE];
  m_reg[MEPHeader::TDS][MEPHeader::TDS_PROTOCOL].size = MEPHeader::TDS_PROTOCOL_SIZE;
  m_reg[MEPHeader::TDS][MEPHeader::TDS_RESULT].addr  = new char[MEPHeader::TDS_RESULT_SIZE];
  m_reg[MEPHeader::TDS][MEPHeader::TDS_RESULT].size  = MEPHeader::TDS_RESULT_SIZE;

  m_reg[MEPHeader::TBB][MEPHeader::TBB_SETTINGSX].addr  = new char[MEPHeader::TBB_SETTINGS_SIZE];
  m_reg[MEPHeader::TBB][MEPHeader::TBB_SETTINGSX].size  = MEPHeader::TBB_SETTINGS_SIZE;
  m_reg[MEPHeader::TBB][MEPHeader::TBB_SETTINGSY].addr  = new char[MEPHeader::TBB_SETTINGS_SIZE];
  m_reg[MEPHeader::TBB][MEPHeader::TBB_SETTINGSY].size  = MEPHeader::TBB_SETTINGS_SIZE;
  m_reg[MEPHeader::TBB][MEPHeader::TBB_BANDSELY].addr  = new char[MEPHeader::TBB_BANDSEL_SIZE];
  m_reg[MEPHeader::TBB][MEPHeader::TBB_BANDSELY].size  = MEPHeader::TBB_BANDSEL_SIZE;
  m_reg[MEPHeader::TBB][MEPHeader::TBB_BANDSELY].addr  = new char[MEPHeader::TBB_BANDSEL_SIZE];
  m_reg[MEPHeader::TBB][MEPHeader::TBB_BANDSELY].size  = MEPHeader::TBB_BANDSEL_SIZE;

  //
  // initialize registers to some sensible test pattern
  //
  for (int pid = MEPHeader::MIN_PID; pid <= MEPHeader::MAX_PID; pid++)
    {
      for (int regid = 0; regid <= MEPHeader::MAX_REGID; regid++)
	{
	  uint16 size = m_reg[pid][regid].size;
	  
	  if (size)
	    {
	      switch (pid)
		{
		case MEPHeader::RSR:
		  // VERSION register is defined for all BLPs
		  if (MEPHeader::RSR_VERSION == regid) {
		    size *= GET_CONFIG("EPAStub.N_BLPS", i);
		  }
		case MEPHeader::RSU:
		case MEPHeader::CDO:
		case MEPHeader::TDS:
		case MEPHeader::TBB:
		  // initialize with 0
		  memset(m_reg[pid][regid].addr, 0, size);
		  break;

		case MEPHeader::SS:
		case MEPHeader::BF:
		case MEPHeader::DIAG:
		case MEPHeader::RCU:
		case MEPHeader::BS:
		  size *= GET_CONFIG("EPAStub.N_BLPS", i);

		  // initialize with 0
		  memset(m_reg[pid][regid].addr, 0, size);
		  break;

		case MEPHeader::CR:
		  size *= GET_CONFIG("EPAStub.N_BLPS", i) + 1;

		  // initialize with 0
		  memset(m_reg[pid][regid].addr, 0, size);
		  break;
	    
		case MEPHeader::SST:
		  if (MEPHeader::SST_POWER == regid)
		    {
		      //
		      // Initialize subband statistics register with sensible test pattern
		      // This is done on a per BLP basis. On a BLP the statistics for
		      // X and Y are interleaved.
		      // The subband statistics plot will have a sine wave for each RCU as a
		      // different level.
		      //
		      uint32* pu_32 = (uint32*)m_reg[pid][regid].addr;
		      for (int blp = 0; blp < GET_CONFIG("EPAStub.N_BLPS", i); blp++) {
			for (uint32 i = 0; i < size / sizeof(uint32); i++) {
			  int rcu = (0 == i % 2 ? blp*2 : blp*2+1);
			  *pu_32++ = (uint32)(std::pow(10.0,
						       1.0+(rcu+1)*1.0
						       +0.5*std::sin(i/(MEPHeader::N_SUBBANDS/4.0)))/10.0); // + (0 == i % 2 ? i : 4096 - i);
			}
		      }
		    }
		  break;

		case MEPHeader::BST:
		    if (0 <= regid < MEPHeader::N_SERDES_LANES)
		    {
		      //
		      // Initialize beamlet statistics register with sensible test pattern
		      // The beamlet statistics plot will two sine waves, one for X and one for Y.
		      //
		      uint32* pu_32 = (uint32*)m_reg[pid][regid].addr;
		      for (uint32 i = 0; i < size / sizeof(uint32); i++) {
			*pu_32++ = (uint32)(std::pow(10.0,
						     1.0+(i%2+1)*1.0
						     +0.5*std::sin(i/(MEPHeader::N_BEAMLETS/4.0)))/10.0); // + (0 == i % 2 ? i : 4096 - i);
		      }
		    }
		  break;

		case MEPHeader::XST:
		  {
		    uint32* pu_32 = (uint32*)m_reg[pid][regid].addr;
		    for (uint32 i = 0; i < size / sizeof(uint32); i++) *pu_32++ = i;
		  }
		  break;
		}
	    }
	}
    }
}

EPAStub::~EPAStub()
{
  for (int pid = MEPHeader::MIN_PID; pid <= MEPHeader::MAX_PID; pid++)
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
      port.setTimer((long)3);
      LOG_WARN(formatString("port '%s' disconnected, retry in 3 seconds...", port.getName().c_str()));
      port.close();
    }
    break;

    case F_TIMER:
    {
      // try again
      LOG_DEBUG(formatString("port '%s' retry of open...", port.getName().c_str()));
      m_client.open();
    }
    break;

    default:
      status = GCFEvent::NOT_HANDLED;
      break;
  }

  return status;
}

static void log_hdr(MEPHeader& hdr)
{
  LOG_ERROR(formatString("F_DATAIN: type=0x%02x, status=%d, frame_length=%d, "
			 "addr=(0x%04x 0x%02x 0x%02x), offset=%d, payload_length=%d, seqnr=%d",
			 hdr.m_fields.type,
			 hdr.m_fields.status,
			 hdr.m_fields.frame_length,
			 hdr.m_fields.addr.dstid,
			 hdr.m_fields.addr.pid,
			 hdr.m_fields.addr.regid,
			 hdr.m_fields.offset,
			 hdr.m_fields.payload_length,
			 hdr.m_fields.seqnr));
}

static uint8 bitindex16(uint8 value)
{
  const uint8 logTable16[] = { 0, 0,
			       1, 1,
			       2, 2, 2, 2,
			       3, 3, 3, 3, 3, 3, 3, 3 };

  return logTable16[value & 0xF];
}

GCFEvent::TResult EPAStub::connected(GCFEvent& event, GCFPortInterface& port)
{
  GCFEvent::TResult status = GCFEvent::HANDLED;
  
  switch (event.signal)
    {
    case F_ENTRY:
      {
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

	uint16 frame_length   = read.hdr.m_fields.frame_length;
	uint16 pid            = read.hdr.m_fields.addr.pid;
	uint8  regid          = read.hdr.m_fields.addr.regid;
	uint16 offset         = read.hdr.m_fields.offset;
	uint16 payload_length = read.hdr.m_fields.payload_length;

	EPAReadackEvent ack;
	ack.hdr = read.hdr;
	ack.hdr.m_fields.type   = MEPHeader::READACK;
	ack.hdr.m_fields.status = 0;
      
	// check consistency of request, if any problem found, ignore the request
	if (   frame_length != MEPHeader::SIZE
	       || pid < MEPHeader::MIN_PID
	       || pid > MEPHeader::MAX_PID
	       || regid > MEPHeader::MAX_REGID
	       || !m_reg[pid][regid].addr) {
	  LOG_ERROR_STR(formatString("Discarding invalid EPA_READ request (frame_length=%d,pid=%d,regid=%d",
				     frame_length, pid, regid));
	  log_hdr(read.hdr);
	  break;
	}

	if (MEPHeader::DST_RSP == read.hdr.m_fields.addr.dstid)
	  {
	    // RSP register read
	    if (offset + payload_length > m_reg[pid][regid].size) {
	      LOG_ERROR("Discarding invalid EPA_READ RSP request (invalid offset/payload_length combination)");
	      log_hdr(read.hdr);
	      break;
	    }
	  }
	else
	  {
	    uint16 dstid = bitindex16(read.hdr.m_fields.addr.dstid & 0x000F); // calculate offset from bit that was set
	    offset += dstid * m_reg[pid][regid].size;
	    if (offset + payload_length > m_reg[pid][regid].size * (uint16)GET_CONFIG("EPAStub.N_BLPS", i)) {
	      LOG_ERROR("Discarding invalid EPA_READ BLP request (invalid offset/payload_length combination)");
	      log_hdr(read.hdr);
	      break;
	    }
	  }
      
	memcpy(ack.data, m_reg[pid][regid].addr + offset, payload_length);
	//ack.payload.setBuffer(m_reg[pid][regid].addr + offset, payload_length);
	ack.hdr.m_fields.frame_length += payload_length;

	port.send(ack);
      }
      break;
      
      //
      // All register write requests arrive as specific signals
      // and are all handled in the same way,
      // copy to register memory and acknowledge with success status.
      //
    case EPA_RSR_STATUS:
    case EPA_RSR_VERSION:
    case EPA_DIAG_RESULTS:
      {
	// log invalid write events
	EPAWriteEvent write(event);

	LOG_ERROR(formatString("Write request on read-only register: PID=0x%02x, REGID=0x%02x",
			       write.hdr.m_fields.addr.pid, write.hdr.m_fields.addr.regid));
	log_hdr(write.hdr);
      }
      break;

    case EPA_RSU_FLASHRW:
    case EPA_RSU_FLASHERASE:
    case EPA_RSU_RECONFIGURE:
    case EPA_RSU_RESET:
    case EPA_DIAG_WG:
    case EPA_DIAG_WGWAVE:
    case EPA_DIAG_BYPASS:
    case EPA_DIAG_SELFTEST:
    case EPA_SS_SELECT:
    case EPA_BF_COEFS_WRITE:
    case EPA_BST_STATS:
    case EPA_SST_STATS:
    case EPA_RCU_SETTINGS:
    case EPA_RCU_PROTOCOL:
    case EPA_RCU_RESULT:
    case EPA_CR_CONTROL:
    case EPA_XST_STATS:
    case EPA_CDO_SETTINGS:
    case EPA_CDO_HEADER:
    case EPA_BS_NOFSAMPLESPERSYNC:
    case EPA_TDS_PROTOCOL:
    case EPA_TDS_RESULT:
    case EPA_TBB_SETTINGS:
    case EPA_TBB_BANDSEL:
      {
	EPAWriteEvent write(event);

	LOG_DEBUG(formatString("Received event (pid=0x%02x, regid=0x%02x)",
			       write.hdr.m_fields.addr.pid,
			       write.hdr.m_fields.addr.regid));


	uint16 frame_length   = write.hdr.m_fields.frame_length;
	uint16 pid            = write.hdr.m_fields.addr.pid;
	uint8  regid          = write.hdr.m_fields.addr.regid;
	uint16 offset         = write.hdr.m_fields.offset;
	uint16 payload_length = write.hdr.m_fields.payload_length;

	if (frame_length != payload_length + MEPHeader::SIZE
	    || pid < MEPHeader::MIN_PID
	    || pid > MEPHeader::MAX_PID
	    || regid > MEPHeader::MAX_REGID
	    || !m_reg[pid][regid].addr) {
	  LOG_ERROR("Discarding invalid EPA_WRITE request");
	  log_hdr(write.hdr);
	  break;
	}
	
	if (MEPHeader::DST_RSP == write.hdr.m_fields.addr.dstid)
	  {
	    // copy to RSP register memory
	    if (offset + payload_length > m_reg[pid][regid].size) {
	      LOG_ERROR("Discarding invalid write RSP request (invalid offset/payload_length combination)");
	      log_hdr(write.hdr);
	      break;
	    }
	  }
	else
	  {
	    // copy to BLP register memory

	    uint8 dstid = bitindex16(write.hdr.m_fields.addr.dstid & 0x000F); // calculate offset from bit that was set
	    offset += dstid * m_reg[pid][regid].size;
	    
	    if (offset + payload_length > m_reg[pid][regid].size * (int16)GET_CONFIG("EPAStub.N_BLPS", i)) {
	      LOG_ERROR("Discarding invalid write BLP request (invalid offset/payload_length combination)");
	      log_hdr(write.hdr);
	      break;
	    }
	  }

	write.payload.setBuffer(m_reg[pid][regid].addr + offset, payload_length); 
	write.payload.unpack((char*)&event
			     + sizeof(GCFEvent)
			     + sizeof(MEPHeader::FieldsType));

	//memcpy(m_reg[pid][regid].addr + offset,
	//       write.payload.getBuffer(), size);

	EPAWriteackEvent writeack;
      
	writeack.hdr = write.hdr; // copy request header
	writeack.hdr.m_fields.type   = MEPHeader::WRITEACK;
	writeack.hdr.m_fields.status = 0;
	writeack.hdr.m_fields.payload_length = payload_length;
	writeack.hdr.m_fields.frame_length = MEPHeader::SIZE;

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
      GCFScheduler::instance()->stop();
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
  rsr_status.hdr.m_fields.type   = MEPHeader::READACK;
  rsr_status.hdr.m_fields.status = 0;

  //memset(&rsr_status.board, 0, sizeof(EPA_Protocol::BoardStatus));
  ASSERT(rsr_status.hdr.m_fields.payload_length <= m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].size);
  memcpy(&rsr_status.board, m_reg[MEPHeader::RSR][MEPHeader::RSR_STATUS].addr, rsr_status.hdr.m_fields.payload_length);

  port.send(rsr_status);

  return GCFEvent::HANDLED;
}

GCFEvent::TResult EPAStub::read_rsr_version(EPAReadEvent& event, GCFPortInterface& port)
{
  EPARsrVersionEvent rsr_version;

  rsr_version.hdr = event.hdr;
  rsr_version.hdr.m_fields.type   = MEPHeader::READACK;
  rsr_version.hdr.m_fields.status = 0;

#if 0
  rsr_version.rsp_version = 0x12;
  rsr_version.bp_version  = 0x34;
  rsr_version.ap_version  = 0x56;
#endif

  port.send(rsr_version);

  return GCFEvent::HANDLED;
}

GCFEvent::TResult EPAStub::read_stats(EPAReadEvent& event, GCFPortInterface& port)
{
  EPASstStatsEvent stats;

  stats.hdr = event.hdr;
  stats.hdr.m_fields.type   = MEPHeader::READACK;
  stats.hdr.m_fields.status = 0;

  for (uint32 i = 0; i < stats.hdr.m_fields.payload_length / sizeof(uint32); i++)
  {
    stats.stat[i] = i + (stats.hdr.m_fields.offset / sizeof(uint32));
  }

  port.send(stats);

  return GCFEvent::HANDLED;
}

void EPAStub::run()
{
  start(); // make initial transition
  GCFScheduler::instance()->run();
}


  } // namespace RSP_Test
} // namespace LOFAR


using namespace LOFAR;
using namespace RSP_Test;
using namespace GCF::TM;

int main(int argc, char** argv)
{
  GCFScheduler::instance()->init(argc, argv);

  LOG_INFO(formatString("Program %s has started", argv[0]));

  EPAStub stub("EPAStub");
  stub.run();

  LOG_INFO("Normal termination of program");

  return 0;
}
