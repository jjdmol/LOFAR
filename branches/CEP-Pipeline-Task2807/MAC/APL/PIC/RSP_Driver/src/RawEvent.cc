//#
//#  RawEvent.cc: implementation of the RawEvent class.
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
#include <Common/StringUtil.h>
#include <Common/hexdump.h>
#include <GCF/TM/GCF_ETHRawPort.h>

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/MEPHeader.h>

#include "RawEvent.h"

using namespace LOFAR;
using namespace GCF::TM;
using namespace EPA_Protocol;

/**
 * Lookup table to map MEP message header (addr.[pid,reg]) to GCFEvent signal.
 */
static unsigned short signal_lut[MEPHeader::MAX_PID + 1][MEPHeader::MAX_REGID + 1][MEPHeader::MAX_TYPE + 1] = 
{
  /* pid = 0x01 (RSR) */
  {
    /* reg = 0x00 (RSR_STATUS) */
    { 0,
      EPA_READ,           /* READ      */
      0,
      EPA_RSR_STATUS,     /* READACK   */
      0,
    },

    /* reg = 0x01 (RSR_VERSION) */
    { 0,
      EPA_READ,           /* READ    */
      0,
      EPA_RSR_VERSION,    /* READACK */
      0,
    },

    /* reg = 0x02 (RSR_TIMESTAMP) */
    { 0,
      EPA_READ,          /* READ     */
      EPA_RSR_TIMESTAMP, /* WRITE    */
      EPA_RSR_TIMESTAMP, /* READACK  */
      EPA_WRITEACK,      /* WRITEACK */
    },
  },

  /* pid = 0x02 (RSU) */
  {
    /**
     * Register with index 0x00 is undefined to make it more difficult
     * to accidentally perform some unintended action on the flash.
     */
    /* reg = 0x00 (UNDEFINED) */
    { 0, 0, 0, 0, 0 },
      
    /* reg = 0x01 (RSU_FLASHRW) */
    { 0,
      EPA_READ,         /* READ */
      EPA_RSU_FLASHRW,  /* WRITE   */
      EPA_RSU_FLASHRW,  /* READACK */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x02 (RSU_FLASHERASE) */
    { 0,
      0,
      EPA_RSU_FLASHERASE, /* WRITE   */
      0,
      EPA_WRITEACK,       /* WRITEACK */
    },

    /* reg = 0x03 (RSU_RECONFIGURE) */
    { 0,
      0,
      EPA_RSU_RECONFIGURE, /* WRITE   */
      0,
      EPA_WRITEACK,        /* WRITEACK */
    },

    /* reg = 0x04 (RSU_RESET) */
    { 0,
      0,
      EPA_RSU_RESET, /* WRITE   */
      0,
      EPA_WRITEACK,       /* WRITEACK */
    },
  },

  /* pid = 0x03 (DIAG) */
  {
    /* reg = 0x00 (DIAG_WGX) */
    { 0,
      EPA_READ,     /* READ */
      EPA_DIAG_WG,  /* WRITE   */
      EPA_DIAG_WG,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },

    /* reg = 0x01 (DIAG_WGY) */
    { 0,
      EPA_READ,     /* READ */
      EPA_DIAG_WG,  /* WRITE   */
      EPA_DIAG_WG,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },

    /* reg = 0x02 (DIAG_WGXWAVE) */
    { 0,
      EPA_READ,        /* READ    */
      EPA_DIAG_WGWAVE, /* WRITE   */
      EPA_DIAG_WGWAVE, /* READACK */
      EPA_WRITEACK,    /* WRITEACK */
    },

    /* reg = 0x03 (DIAG_WGYWAVE) */
    { 0,
      EPA_READ,        /* READ    */
      EPA_DIAG_WGWAVE, /* WRITE   */
      EPA_DIAG_WGWAVE, /* READACK */
      EPA_WRITEACK,    /* WRITEACK */
    },
    /* reg = 0x04 (DIAG_WGBYPASS) */
    { 0,
      EPA_READ,        /* READ    */
      EPA_DIAG_BYPASS, /* WRITE   */
      EPA_DIAG_BYPASS, /* READACK */
      EPA_WRITEACK,    /* WRITEACK */
    },
    /* reg = 0x05 (DIAG_RESULTS) */
    { 0,
      EPA_READ,         /* READ    */
      0,
      EPA_DIAG_RESULTS, /* READACK */
      0,
    },
    /* reg = 0x06 (DIAG_SELFTEST) */
    { 0,
      EPA_READ,          /* READ    */
      EPA_DIAG_SELFTEST, /* WRITE   */
      EPA_DIAG_SELFTEST, /* READACK */
      EPA_WRITEACK,      /* WRITEACK */
    },
  },

  /* pid = 0x04 (SS) */
  {
    /* reg = 0x00 (Subband Select parameters) */
    { 0,
      EPA_READ,      /* READ    */
      EPA_SS_SELECT, /* WRITE   */
      EPA_SS_SELECT, /* READACK */
      EPA_WRITEACK,  /* WRITEACK */
    },
  },

  /* pid = 0x05 (BF) */
  {
    /* reg = 0x00 (BF_XROUT) */
    { 0,
      EPA_READ,           /* READ    */
      EPA_BF_COEFS_WRITE, /* WRITE   */
      EPA_BF_COEFS_READ,  /* READACK */
      EPA_WRITEACK,       /* WRITEACK */
    },
    /* reg = 0x01 (BF_XIOUT) */
    { 0,
      EPA_READ,           /* READ    */
      EPA_BF_COEFS_WRITE, /* WRITE   */
      EPA_BF_COEFS_READ,  /* READACK */
      EPA_WRITEACK,       /* WRITEACK */
    },
    /* reg = 0x02 (BF_YROUT) */
    { 0,
      EPA_READ,           /* READ    */
      EPA_BF_COEFS_WRITE, /* WRITE   */
      EPA_BF_COEFS_READ,  /* READACK */
      EPA_WRITEACK,       /* WRITEACK */
    },
    /* reg = 0x03 (BF_YIOUT) */
    { 0,
      EPA_READ,           /* READ    */
      EPA_BF_COEFS_WRITE, /* WRITE   */
      EPA_BF_COEFS_READ,  /* READACK */
      EPA_WRITEACK,       /* WRITEACK */
    },
  },

  /* pid = 0x06 (BST) */
  {
    /* reg = 0x00 (BST_POWER_LANE_0) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_BST_STATS, /* READACK */
      0,
    },
    /* reg = 0x01 (BST_POWER_LANE_1) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_BST_STATS, /* READACK */
      0,
    },
    /* reg = 0x02 (BST_POWER_LANE_2) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_BST_STATS, /* READACK */
      0,
    },
    /* reg = 0x03 (BST_POWER_LANE_3) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_BST_STATS, /* READACK */
      0,
    },
  },

  /* pid = 0x07 (SST) */
  {
    /* reg = 0x00 (SS_POWER) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_SST_STATS, /* READACK */
      0,
    },
  },

  /* pid = 0x08 (RCU) */
  {
    /* reg = 0x00 (RCU_SETTINGS) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_RCU_SETTINGS, /* WRITE    */
      EPA_RCU_SETTINGS, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x01 (RCU_PROTOCOLX) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_RCU_PROTOCOL, /* WRITE    */
      EPA_RCU_PROTOCOL, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x02 (RCU_RESULTX) */
    { 0,
      EPA_READ,       /* READ     */
      EPA_RCU_RESULT, /* WRITE    */
      EPA_RCU_RESULT, /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },

    /* reg = 0x03 (RCU_PROTOCOLY) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_RCU_PROTOCOL, /* WRITE    */
      EPA_RCU_PROTOCOL, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x04 (RCU_RESULTY) */
    { 0,
      EPA_READ,       /* READ     */
      EPA_RCU_RESULT, /* WRITE    */
      EPA_RCU_RESULT, /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },
  },

  /* pid = 0x09 (CR) */
  {
    /* reg = 0x00 (CR_SOFTCLEAR) */
    { 0,
      0,
      EPA_CR_CONTROL, /* WRITE    */
      0,
      EPA_WRITEACK,   /* WRITEACK */
    },
    /* reg = 0x01 (CR_SOFTSYNC) */
    { 0,
      0,
      EPA_CR_CONTROL, /* WRITE    */
      0,
      EPA_WRITEACK,   /* WRITEACK */
    },
    /* reg = 0x02 (CR_SYNCDISABLE) */
    { 0,
      EPA_READ,       /* READ	  */
      EPA_CR_CONTROL, /* WRITE    */
      EPA_CR_CONTROL, /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },
    /* reg = 0x03 (CR_SYNCDELAY) */
    { 0,
      EPA_READ,       /* READ	  */
      EPA_CR_CONTROL, /* WRITE    */
      EPA_CR_CONTROL, /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },
  },

  /* pid = 0x0A (XST) */
  {
    /* reg = 0x00 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x01 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x02 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x03 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x04 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x05 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x06 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x07 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x08 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x09 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x0A */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x0B */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x0C */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x0D */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x0E */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x0F */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x10 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x11 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x12 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x13 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x14 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x15 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x16 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x17 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x18 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x19 */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x1A */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x1B */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x1C */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x1D */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x1E */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x1F */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
  },

  /* pid = 0x0B (CDO) */
  {
    /* reg = 0x00 (CDO_SETTINGS) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_CDO_SETTINGS, /* WRITE    */
      EPA_READACK,      /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },
    /* reg = 0x01 (CDO_HEADER) */
    { 0,
      EPA_READ,       /* READ     */
      EPA_CDO_HEADER, /* WRITE    */
      EPA_READACK,    /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },
  },

  /* pid = 0x0C (BS) */
  {
    /* reg = 0x00 (BS_NRSAMPLES) */
    { 0,
      EPA_READ,                 /* READ     */
      EPA_BS_NOFSAMPLESPERSYNC, /* WRITE    */
      EPA_BS_NOFSAMPLESPERSYNC, /* READACK  */
      EPA_WRITEACK,             /* WRITEACK */
    },
  },

  /* pid = 0x0D (SERDES) */
  {
    /* reg = 0x00 (MDIO_HEADER) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_READACK,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    
    /* reg = 0x01 (MDIO_DATA) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_MDIO_DATA,/* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x0E (TDS) */
  {
    /* reg = 0x00 (TDS_PROTOCOL) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_TDS_PROTOCOL, /* WRITE    */
      EPA_TDS_PROTOCOL, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x01 (TDS_RESULT) */
    { 0,
      EPA_READ,       /* READ     */
      EPA_TDS_RESULT, /* WRITE    */
      EPA_TDS_RESULT, /* READACK  */
      EPA_WRITEACK,   /* WRITEACK */
    },
  },

  /* pid = 0x0F (TBB) */
  {
    /* reg = 0x00 (TBB_SETTINGSX) */
    { 0,
      0,
      EPA_TBB_SETTINGS, /* WRITE    */
      0,
      EPA_WRITEACK,    /* WRITEACK */
    },
    /* reg = 0x01 (TBB_SETTINGSY) */
    { 0,
      0,
      EPA_TBB_SETTINGS, /* WRITE    */
      0,
      EPA_WRITEACK,    /* WRITEACK */
    },
    /* reg = 0x02 (TBB_BANDSELX) */
    { 0,
      0,
      EPA_TBB_BANDSEL, /* WRITE    */
      0,
      EPA_WRITEACK,    /* WRITEACK */
    },
    /* reg = 0x03 (TBB_BANDSELY) */
    { 0,
      0,
      EPA_TBB_BANDSEL, /* WRITE    */
      0,
      EPA_WRITEACK,    /* WRITEACK */
    },
  },

  /* pid = 0x10 (CEP) */
  {
    /* reg = 0x00 (MDIO_HEADER) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_READACK,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    
    /* reg = 0x01 (MDIO_DATA) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_READACK,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x11 (LCU) */
  {
    /* reg = 0x00 (MDIO_HEADER) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_READACK,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    
    /* reg = 0x01 (MDIO_DATA) */
    { 0,
      EPA_READ,     /* READ */
      EPA_WRITE,    /* WRITE */
      EPA_READACK,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x12 (RAD) */
  {
    /* reg = 0x00 (RAD_SETTINGS) */
    { 0,
      EPA_READ,		/* READ */
      EPA_RAD_SET,  /* WRITE */
      EPA_RAD_SET,	/* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x01 (RAD_LATENCY) */
    { 0,
      EPA_READ,		/* READ */
      EPA_RAD_LAT,  /* WRITE */
      EPA_RAD_LAT,	/* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },


};

typedef struct {
  GCFEvent              event;
  MEPHeader::FieldsType mephdr;
  uint8                 payload[ETH_DATA_LEN];
} RawFrame;
static RawFrame buf;

GCFEvent::TResult RawEvent::dispatch(GCFTask& task, GCFPortInterface& port)
{

#if 0
	cout << " sizeof(RawFrame)              =" << sizeof(RawFrame) << endl
		 << " sizeof(GCFEvent)              =" << sizeof(GCFEvent) << endl
		 << " sizeof(MEPHeader::FieldsType) =" << sizeof(MEPHeader::FieldsType) << endl
		 << " sizeof(payload)               =" << sizeof(buf.payload) << endl;
#endif

	GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

	// Receive a raw packet
	ssize_t size = port.recv(&buf.mephdr, ETH_DATA_LEN);

	// always need at least header_size bytes.
	if (size < (ssize_t)sizeof(buf.mephdr)) {
		return GCFEvent::NOT_HANDLED;
	}

	LOG_DEBUG(formatString("F_DATAIN: type=0x%02x, status=%d, frame_length=%d, "
					"addr=(0x%04x 0x%02x 0x%02x), payload_length=%d, seqnr=%d",
					buf.mephdr.type,
					buf.mephdr.status,
					buf.mephdr.frame_length,
					buf.mephdr.addr.dstid,
					buf.mephdr.addr.pid,
					buf.mephdr.addr.regid,
					buf.mephdr.payload_length,
					buf.mephdr.seqnr));

	unsigned short signal = 0; // signal == 0 indicates unrecognised or invalid MEP message

	//
	// Decode the header fields
	//
	if (   buf.mephdr.type       >= MEPHeader::MIN_TYPE
		&& buf.mephdr.type       <= MEPHeader::MAX_TYPE
		&& buf.mephdr.addr.pid   >= MEPHeader::MIN_PID
		&& buf.mephdr.addr.pid   <= MEPHeader::MAX_PID
		&& buf.mephdr.addr.regid <= MEPHeader::MAX_REGID) {
		/* always true due to limited range of datatype
		&& buf.mephdr.addr.regid >= MEPHeader::MIN_REGID */
		//
		// If no error, lookup signal number, else assign ACK_ERROR signal number
		//
		if (buf.mephdr.status == 0) {
			signal = signal_lut[buf.mephdr.addr.pid - MEPHeader::MIN_PID][buf.mephdr.addr.regid][buf.mephdr.type];
		}
		else {
			if (MEPHeader::READACK == buf.mephdr.type) {
				signal = EPA_READACK_ERROR;
			}
			else if (MEPHeader::WRITEACK == buf.mephdr.type) {
				signal = EPA_WRITEACK_ERROR;
			}
			else {
				LOG_WARN("Protocol violation: received message other than MEPHeader::READACK or MEPHeader::WRITEACK with error != 0 set.");
			}
		} // status
	} else {
		LOG_WARN("Received message with out-of-range header fields");
		string	hDump;
		hexdump (hDump, (char*)&buf.mephdr, sizeof(buf.mephdr));
		LOG_INFO (hDump);
	}

	if (signal) { // signal == 0 indicates unrecognised or invalid MEP message
//		(void)new((void*)&buf.event) GCFEvent(signal); // placement new does in place construction

		// check if there is more data than needed
//		if (size - (sizeof(GCFEvent) + sizeof(MEPHeader::FieldsType)) > 0) {
//			LOG_DEBUG(formatString("discarding %d bytes", size - (sizeof(GCFEvent) + sizeof(MEPHeader::FieldsType))));
//		}

		//
		// Print debugging info
		// 
#if 0
		if ((F_DATAIN != buf.event.signal) && 
			(F_DATAOUT != buf.event.signal) &&
			(F_EVT_PROTOCOL(buf.event) != F_FSM_PROTOCOL) &&
			(F_EVT_PROTOCOL(buf.event) != F_PORT_PROTOCOL)) {
				LOG_DEBUG(formatString("%s receives '%s' on port '%s'",
				task.getName().c_str(), 
				task.evtstr(buf.event), port.getName().c_str()));
		}
#endif

		//
		// dispatch the MEP message as a GCFEvent (which it now is)
		//
		buf.event.signal  = signal;
		buf.event.length  = sizeof(buf.mephdr) + buf.mephdr.payload_length;
		buf.event._buffer = (char*)(&buf.mephdr) - GCFEvent::sizePackedGCFEvent;

		status = task.doEvent(buf.event, port);

		buf.event._buffer = 0L;
		buf.event.length = 0;
	}
	else {
		LOG_WARN("F_DATAIN: Discarding unknown message.");
	}

	return (status);
}
 

