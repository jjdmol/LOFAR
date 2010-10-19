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
#include <GCF/TM/GCF_ETHRawPort.h>

#include <APL/RSP_Protocol/EPA_Protocol.ph>
#include <APL/RSP_Protocol/MEPHeader.h>

#include "RawEvent.h"

using namespace LOFAR;
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
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x01 (BF_XIOUT) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x02 (BF_YROUT) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x03 (BF_YIOUT) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x06 (BST) */
  {
    /* reg = 0x00 (BST_POWER) */
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

    /* reg = 0x02 (RCU_RESULTSX) */
    { 0,
      EPA_READ,        /* READ     */
      0,
      EPA_RCU_RESULTS, /* READACK  */
      0,
    },

    /* reg = 0x03 (RCU_PROTOCOLY) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_RCU_PROTOCOL, /* WRITE    */
      EPA_RCU_PROTOCOL, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x04 (RCU_RESULTSY) */
    { 0,
      EPA_READ,        /* READ     */
      0,
      EPA_RCU_RESULTS, /* READACK  */
      0,
    },
  },

  /* pid = 0x09 (CR) */
  {
    /* reg = 0x00 (CR_CONTROL) */
    { 0,
      0,
      EPA_CR_CONTROL, /* WRITE    */
      0,
      EPA_WRITEACK,   /* WRITEACK */
    },

    /* reg = 0x01 (Header) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_CDO_HEADER, /* WRITE    */
      EPA_CDO_HEADER, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },
  },

  /* pid = 0x0A (XST) */
  {
    /* reg = 0x00 (XST_0_X) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x01 (XST_0_Y) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x02 (XST_1_X) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x03 (XST_1_Y) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x04 (XST_2_X) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x05 (XST_2_Y) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },
    /* reg = 0x06 (XST_3_X) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_XST_STATS, /* READACK */
      0,
    },

    /* reg = 0x07 (XST_3_Y) */
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
      EPA_CDO_SETTINGS, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
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

  /* pid = 0x0D (RESERVED) */
  {
    /* RESERVED */
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

    /* reg = 0x01 (TDS_RESULTS) */
    { 0,
      EPA_READ,         /* READ     */
      0,
      EPA_TDS_PROTOCOL, /* READACK  */
      0,
    },
  },

  /* pid = 0x0F (TBB) */
  {
    /* reg = 0x00 (TBB_CONTROL) */
    { 0,
      0,
      EPA_TBB_CONTROL, /* WRITE    */
      0,
      EPA_WRITEACK,    /* WRITEACK */
    },
  },

};

GCFEvent::TResult RawEvent::dispatch(GCFTask& task, GCFPortInterface& port)
{
  static char buf[ETH_DATA_LEN]; // static packet buffer

  GCFEvent::TResult status = GCFEvent::NOT_HANDLED;

  MEPHeader hdr;

  //
  // Receive the packet
  // 
  ssize_t size = port.recv(buf, ETH_DATA_LEN);
  ssize_t offset = 0;

  if (size < (ssize_t)sizeof(hdr.m_fields)) return GCFEvent::NOT_HANDLED;

  //
  // Copy the header
  //
  memcpy(&hdr.m_fields, buf + offset, sizeof(hdr.m_fields));
  offset += sizeof(hdr.m_fields);

  LOG_DEBUG(formatString("F_DATAIN: type=0x%02x, status=%d, frame_length=%d, "
			 "addr=(0x%04x 0x%02x 0x%02x), offset=%d, payload_length=%d, "
			 "seqnr=%d",
			 hdr.m_fields.type,
			 hdr.m_fields.status,
			 hdr.m_fields.frame_length,
			 hdr.m_fields.addr.dstid,
			 hdr.m_fields.addr.pid,
			 hdr.m_fields.addr.regid,
			 hdr.m_fields.offset,
			 hdr.m_fields.payload_length,
			 hdr.m_fields.seqnr));

  unsigned short signal = 0; // signal == 0 indicates unrecognised or invalid MEP message

  //
  // Decode the header fields
  //
  if (   hdr.m_fields.addr.pid   >= MEPHeader::MIN_PID
      && hdr.m_fields.addr.pid   <= MEPHeader::MAX_PID
      && hdr.m_fields.addr.regid <= MEPHeader::MAX_REGID
      && hdr.m_fields.type       <= MEPHeader::MAX_TYPE)
  {
    //
    // If no error, lookup signal number, else assign ACK_ERROR signal number
    //
    if (0 == hdr.m_fields.status)
    {
      signal = signal_lut[hdr.m_fields.addr.pid - MEPHeader::MIN_PID][hdr.m_fields.addr.regid][hdr.m_fields.type];
    }
    else 
    {
      if (MEPHeader::READACK == hdr.m_fields.type) signal = EPA_READACK_ERROR;
      else if (MEPHeader::WRITEACK == hdr.m_fields.type) signal = EPA_WRITEACK_ERROR;
      else 
      {
	LOG_WARN("Protocol violation: received message other than MEPHeader::READACK or MEPHeader::WRITEACK with error != 0 set.");
      }
    }
  }
  
  if (signal) // signal == 0 indicates unrecognised or invalid MEP message
  {
    GCFEvent e(signal);

    //
    // allocate memory for the GCFEvent header,
    // the MEPHeader and the MEP payload.
    //
    GCFEvent* event = (GCFEvent*)new char[sizeof(GCFEvent)
					  + sizeof(hdr.m_fields)
					  + hdr.m_fields.payload_length];

    //
    // Copy the correct event and set the length
    //
    memcpy(event, &e, sizeof(GCFEvent));
    event->length = sizeof(hdr.m_fields) + hdr.m_fields.payload_length;
    memcpy((char*)event + sizeof(GCFEvent), &hdr.m_fields, sizeof(hdr.m_fields));

    //
    // If there was an error, there won't be a payload
    // despite what the size field suggests.
    //
    if (0 == hdr.m_fields.status)
    {
      //
      // Get the payload
      //
      if (size - offset >= hdr.m_fields.payload_length)
      {
	memcpy((char*)event + sizeof(GCFEvent) + sizeof(hdr.m_fields), buf + offset, hdr.m_fields.payload_length);
	offset += hdr.m_fields.payload_length;
      }
    }
    
    if (size - offset > 0)
    {
      LOG_DEBUG(formatString("discarding %d bytes", size - offset));
    }

    //
    // Print debugging info
    // 
    if ((F_DATAIN != event->signal) && 
	(F_DATAOUT != event->signal) &&
	(F_EVT_PROTOCOL(*event) != F_FSM_PROTOCOL) &&
	(F_EVT_PROTOCOL(*event) != F_PORT_PROTOCOL))
    {
      LOG_DEBUG(formatString("%s receives '%s' on port '%s'",
			     task.getName().c_str(), 
			     task.evtstr(*event), port.getName().c_str()));
    }

    //
    // dispatch the MEP message as a GCFEvent (which it now is)
    //
    status = task.dispatch(*event, port);
	
    //
    // Free the memory for the event
    //
    delete [] (char*)event;
  }
  else
  {
    LOG_DEBUG("F_DATAIN: Discarding unknown message.");
  }

  return status;
}
 

