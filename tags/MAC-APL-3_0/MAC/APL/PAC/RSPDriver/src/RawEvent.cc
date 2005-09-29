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

#include "EPA_Protocol.ph"
#include "RawEvent.h"
#include "MEPHeader.h"
#include <GCF/TM/GCF_ETHRawPort.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace LOFAR;
using namespace EPA_Protocol;

/**
 * Lookup table to map MEP message header (addr.[pid,reg]) to GCFEvent signal.
 */
static unsigned short signal_lut[MEPHeader::MAX_PID + 1][MEPHeader::MAX_REGID + 1][MEPHeader::MAX_TYPE + 1] = 
{
  /* pid = 0x00 (STATUS) */
  {
    /* reg = 0x00 (RSP Status) */
    { 0,
      EPA_READ,           /* READ      */
      0,
      EPA_RSR_STATUS,     /* READACK   */
      0,
    },

    /* reg = 0x01 (Version) */
    { 0,
      EPA_READ,           /* READ    */
      0,
      EPA_RSR_VERSION,    /* READACK */
      0,
    },
  },

  /* pid = 0x01 (TST) */
  {
    /* reg = 0x00 (Selftest) */
    { 0,
      EPA_READ,         /* READ */
      EPA_TST_SELFTEST, /* WRITE   */
      EPA_TST_SELFTEST, /* READACK */
      EPA_WRITEACK,     /* WRITEACK */
    },
  },

  /* pid = 0x02 (CFG) */
  {
    /* reg = 0x00 (Reset) */
    { 0,
      EPA_READ,         /* READ */
      EPA_CFG_RESET,    /* WRITE   */
      EPA_CFG_RESET,    /* READACK */
      EPA_WRITEACK,     /* WRITEACK */
    },

    /* reg = 0x01 (Reprogram) */
    { 0,
      EPA_READ,          /* READ */
      EPA_CFG_REPROGRAM, /* WRITE   */
      EPA_CFG_REPROGRAM, /* READACK */
      EPA_WRITEACK,      /* WRITEACK */
    },
  },

  /* pid = 0x03 (WG) */
  {
    /* reg = 0x00 (Waveform generator settings X polarization) */
    { 0,
      EPA_READ,        /* READ */
      EPA_WG_SETTINGS, /* WRITE   */
      EPA_WG_SETTINGS, /* READACK */
      EPA_WRITEACK,    /* WRITEACK */
    },

    /* reg = 0x01 (Waveform generator settings Y polarization) */
    { 0,
      EPA_READ,       /* READ */
      EPA_WG_SETTINGS,/* WRITE   */
      EPA_WG_SETTINGS,/* READACK */
      EPA_WRITEACK,   /* WRITEACK */
    },

    /* reg = 0x02 (User waveform X polarization) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_WG_WAVE,  /* WRITE   */
      EPA_WG_WAVE,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },

    /* reg = 0x03 (User waveform Y polarization) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_WG_WAVE,  /* WRITE   */
      EPA_WG_WAVE,  /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x04 (SS) */
  {
    /* reg = 0x00 (Subband Select parameters) */
    { 0,
      EPA_READ,      /* READ    */
      EPA_SS_SELECT, /* WRITE   */
      EPA_SS_SELECT, /* READRES */
      EPA_WRITEACK,  /* READERR */
    },
  },

  /* pid = 0x05 (BF) */
  {
    /* reg = 0x00 (Coefs XR output) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x01 (Coefs XI output) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x02 (Coefs YR output ) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
    /* reg = 0x03 (Coefs YI output) */
    { 0,
      EPA_READ,     /* READ    */
      EPA_BF_COEFS, /* WRITE   */
      EPA_BF_COEFS, /* READACK */
      EPA_WRITEACK, /* WRITEACK */
    },
  },

  /* pid = 0x06 (BST) */
  {
    /* reg = 0x00 (Mean) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_STATS,     /* READACK */
      0,
    },

    /* reg = 0x01 (Power) */
    { 0,
      EPA_READ,      /* READ    */
      0,
      EPA_STATS,     /* READACK */
      0,
    },
  },

  /* pid = 0x07 (SST) */
  {
    /* reg = 0x00 (Mean) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_STATS,     /* READACK */
      0,
    },

    /* reg = 0x01 (Power) */
    { 0,
      EPA_READ,      /* READ */
      0,
      EPA_STATS,     /* READACK */
      0,
    },
  },

  /* pid = 0x08 (RCU) */
  {
    /* reg = 0x00 (RCU Settings) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_RCU_SETTINGS, /* WRITE    */
      EPA_RCU_SETTINGS, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
    },
  },

  /* pid = 0x09 (CRR) */
  {
    /* reg = 0x00 (Soft Reset) */
    { 0,
      EPA_READ,          /* READ     */
      EPA_CRR_SOFTRESET, /* WRITE    */
      EPA_CRR_SOFTRESET, /* READACK  */
      EPA_WRITEACK,      /* WRITEACK */
    },

    /* reg = 0x01 (Soft PPS) */
    { 0,
      EPA_READ,          /* READ     */
      EPA_CRR_SOFTPPS,   /* WRITE    */
      EPA_CRR_SOFTPPS,   /* READACK  */
      EPA_WRITEACK,      /* WRITEACK */
    },
  },

  /* pid = 0x0A (CRB) */
  {
    /* reg = 0x00 (Soft Reset) */
    { 0,
      EPA_READ,          /* READ     */
      EPA_CRB_SOFTRESET, /* WRITE    */
      EPA_CRB_SOFTRESET, /* READACK  */
      EPA_WRITEACK,      /* WRITEACK */
    },

    /* reg = 0x01 (Soft PPS) */
    { 0,
      EPA_READ,          /* READ     */
      EPA_CRB_SOFTPPS,   /* WRITE    */
      EPA_CRB_SOFTPPS,   /* READACK  */
      EPA_WRITEACK,      /* WRITEACK */
    },
  },

  /* pid = 0x0B (CDO) */
  {
    /* reg = 0x00 (Settings) */
    { 0,
      EPA_READ,         /* READ     */
      EPA_CDO_SETTINGS, /* WRITE    */
      EPA_CDO_SETTINGS, /* READACK  */
      EPA_WRITEACK,     /* WRITEACK */
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

  LOG_DEBUG(formatString("F_DATAIN: type=0x%02x, error=%d, seqnr=%d, addr=(0x%02x 0x%02x 0x%02x), size=%d",
			 hdr.m_fields.type,
			 hdr.m_fields.error,
			 hdr.m_fields.seqnr,
			 hdr.m_fields.addr.dstid,
			 hdr.m_fields.addr.pid,
			 hdr.m_fields.addr.regid,
			 hdr.m_fields.size));

  unsigned short signal = 0; // signal == 0 indicates unrecognised or invalid MEP message

  //
  // Decode the header fields
  //
  if (   hdr.m_fields.addr.pid   <= MEPHeader::MAX_PID
      && hdr.m_fields.addr.regid <= MEPHeader::MAX_REGID
      && hdr.m_fields.type       <= MEPHeader::MAX_TYPE)
  {
    //
    // If no error, lookup signal number, else assign ACK_ERROR signal number
    //
    if (0 == hdr.m_fields.error)
    {
      signal = signal_lut[hdr.m_fields.addr.pid][hdr.m_fields.addr.regid][hdr.m_fields.type];      
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
					  + hdr.m_fields.size];

    //
    // Copy the correct event and set the length
    //
    memcpy(event, &e, sizeof(GCFEvent));
    event->length = sizeof(hdr.m_fields) + hdr.m_fields.size;
    memcpy((char*)event + sizeof(GCFEvent), &hdr.m_fields, sizeof(hdr.m_fields));

    //
    // If there was an error, there won't be a payload
    // despite what the size field suggests.
    //
    if (0 == hdr.m_fields.error)
    {
      //
      // Get the payload
      //
      if (size - offset >= hdr.m_fields.size)
      {
	memcpy((char*)event + sizeof(GCFEvent) + sizeof(hdr.m_fields), buf + offset, hdr.m_fields.size);
	offset += hdr.m_fields.size;
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
 

