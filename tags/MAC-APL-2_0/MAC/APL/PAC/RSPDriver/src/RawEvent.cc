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

#include "RawEvent.h"
#include "MEPHeader.h"
#include "EPA_Protocol.ph"
#include <GCF/GCF_ETHRawPort.h>

#undef PACKAGE
#undef VERSION
#include <lofar_config.h>
#include <Common/LofarLogger.h>

using namespace EPA_Protocol;
using namespace LOFAR;

/**
 * Lookup table to map MEP message header to GCFEvent signal.
 */
static unsigned short signal_lut[MEPHeader::MAX_PID + 1][MEPHeader::MAX_REGID + 1][MEPHeader::MAX_TYPE + 1] = 
{
  /* pid = 0x00 (STATUS) */
  {
    /* reg = 0x00 (RSP Status) */
    { 0,
      EPA_RSPSTATUS_READ, /* READ    */
      EPA_RSPSTATUS,      /* WRITE   */
      EPA_RSPSTATUS,      /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (Version) */
    { 0,
      EPA_FWVERSION_READ, /* READ    */
      0,                  /* WRITE   */
      EPA_FWVERSION,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x01 (TST) */
  {
    /* reg = 0x00 (Selftest) */
    { 0,
      0,            /* READ    */
      EPA_SELFTEST, /* WRITE   */
      0,            /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x02 (CFG) */
  {
    /* reg = 0x00 (Reset) */
    { 0,
      0,         /* READ    */
      EPA_RESET, /* WRITE   */
      0,         /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (Reprogram) */
    { 0,
      0,             /* READ    */
      EPA_REPROGRAM, /* WRITE   */
      0,             /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x03 (WG) */
  {
    /* reg = 0x00 (Waveform generator settings) */
    { 0,
      EPA_WGSETTINGS_READ, /* READ    */
      EPA_WGSETTINGS,      /* WRITE   */
      EPA_WGSETTINGS,      /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (User waveform) */
    { 0,
      EPA_WGUSER_READ, /* READ    */
      EPA_WGUSER,      /* WRITE   */
      EPA_WGUSER,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x04 (SS) */
  {
    /* reg = 0x00 (Number of selected subbands) */
    { 0,
      EPA_NRSUBBANDS_READ, /* READ    */
      EPA_NRSUBBANDS,      /* WRITE   */
      EPA_NRSUBBANDS,      /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (Subband Select parameters) */
    { 0,
      EPA_SUBBANDSELECT_READ, /* READ    */
      EPA_SUBBANDSELECT,      /* WRITE   */
      EPA_SUBBANDSELECT,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x05 (BF) */
  {
    /* reg = 0x00 (Coefs Xre) */
    { 0,
      EPA_BFCOEFS_READ, /* READ    */
      EPA_BFCOEFS,      /* WRITE   */
      EPA_BFCOEFS,      /* READRES */
      EPA_READERR, /* READERR */
    },
    /* reg = 0x01 (Coefs Xim) */
    { 0,
      EPA_BFCOEFS_READ, /* READ    */
      EPA_BFCOEFS,      /* WRITE   */
      EPA_BFCOEFS,      /* READRES */
      EPA_READERR, /* READERR */
    },
    /* reg = 0x02 (Coefs Yre) */
    { 0,
      EPA_BFCOEFS_READ, /* READ    */
      EPA_BFCOEFS,      /* WRITE   */
      EPA_BFCOEFS,      /* READRES */
      EPA_READERR, /* READERR */
    },
    /* reg = 0x03 (Coefs Yim) */
    { 0,
      EPA_BFCOEFS_READ, /* READ    */
      EPA_BFCOEFS,      /* WRITE   */
      EPA_BFCOEFS,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x06 (ST) */
  {
    /* reg = 0x00 (Mean) */
    { 0,
      EPA_STSTATS_READ, /* READ    */
      EPA_STSTATS,      /* WRITE   */
      EPA_STSTATS,      /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (Power) */
    { 0,
      EPA_STSTATS_READ, /* READ    */
      EPA_STSTATS,      /* WRITE   */
      EPA_STSTATS,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x07 (STSUB) */
  {
    /* reg = 0x00 (Mean) */
    { 0,
      EPA_STSUBSTATS_READ, /* READ    */
      EPA_STSUBSTATS,      /* WRITE   */
      EPA_STSUBSTATS,      /* READRES */
      EPA_READERR, /* READERR */
    },

    /* reg = 0x01 (Power) */
    { 0,
      EPA_STSUBSTATS_READ, /* READ    */
      EPA_STSUBSTATS,      /* WRITE   */
      EPA_STSUBSTATS,      /* READRES */
      EPA_READERR, /* READERR */
    },
  },

  /* pid = 0x08 (RCU) */
  {
    /* reg = 0x00 (RCU Settings) */
    { 0,
      EPA_RCUSETTINGS_READ, /* READ    */
      EPA_RCUSETTINGS,      /* WRITE   */
      EPA_RCUSETTINGS,      /* READRES */
      EPA_READERR, /* READERR */
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

  LOG_DEBUG(formatString("F_DATAIN: type=0x%02x, addr=(0x%02x 0x%02x 0x%02x 0x%02x), size=%d",
			 hdr.m_fields.type,
			 hdr.m_fields.addr.dstid,
			 hdr.m_fields.addr.pid,
			 hdr.m_fields.addr.regid,
			 hdr.m_fields.addr.pageid,
			 hdr.m_fields.size));

  //
  // Decode the header fields
  //
  unsigned short signal = 0;
  if (   hdr.m_fields.addr.pid   <= MEPHeader::MAX_PID
      && hdr.m_fields.addr.regid <= MEPHeader::MAX_REGID
      && hdr.m_fields.type       <= MEPHeader::MAX_TYPE)
  {
    signal = signal_lut[hdr.m_fields.addr.pid][hdr.m_fields.addr.regid][hdr.m_fields.type];
  }
  
  if (signal) // status == 0 indicates unrecognised or invalid MEP message
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
    // Get the payload
    //
#if 0
    size = port.recv((char*)event + sizeof(GCFEvent) + sizeof(hdr.m_fields), hdr.m_fields.size);
#endif
    if (size - offset >= hdr.m_fields.size)
    {
      memcpy((char*)event + sizeof(GCFEvent) + sizeof(hdr.m_fields), buf + offset, hdr.m_fields.size);
      offset += hdr.m_fields.size;
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
    LOG_WARN("F_DATAIN: Discarding unknown message.");
  }

  return status;
}
 

